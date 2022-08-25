#pragma once
#include <iostream>
#include <type_traits>
#include <kayak/gpu_support.hpp>
#include <kayak/tree_layout.hpp>

namespace herring {

namespace detail {

template<typename threshold_t, typename index_t, typename metadata_storage_t, typename offset_t>
auto constexpr get_node_alignment() {
  auto total = (
    std::max(sizeof(threshold_t), sizeof(index_t))
    + sizeof(metadata_storage_t)
    + sizeof(offset_t)
  );
  auto result = std::size_t{8};
  if (total > result) {
    result = std::size_t{16};
  }
  if (total > result) {
    result = std::size_t{32};
  }
  if (total > result) {
    result = std::size_t{64};
  }
  if (total > result) {
    result = std::size_t{128};
  }
  if (total > result) {
    result = std::size_t{256};
  }
  if (total > result) {
    result = total;
  }
  return result;
}

}

/** @brief A single node in a forest model
 *
 * Note that this implementation includes NO error checking for poorly-chosen
 * template types. If the types are not large enough to hold the required data,
 * an incorrect node will be silently constructed. Error checking occurs
 * instead at the time of construction of the entire forest.
 *
 * @tparam layout_v The layout for nodes within the forest
 *
 * @tparam threshold_t The type used as a threshold for evaluating a non-leaf
 * node or (when possible) the output of a leaf node. For non-categorical
 * nodes, if an input value is less than this threshold, the node evaluates to
 * true. For leaf nodes, output values will only be stored as this type if it
 * matches the leaf output type expected by the forest. Typically, this type is
 * either float or double.
 *
 * @tparam index_t The type used as an index to the output data for leaf nodes,
 * or to the categorical set for a categorical non-leaf node. This type should
 * be large enough to index the entire array of output data or categorical sets
 * stored in the forest. Typically, this type is either uint32_t or uint64_t.
 * Smaller types offer no benefit, since this value is stored in a union with
 * threshold_t, which is at least 32 bits.
 *
 * @tparam metadata_storage_t An unsigned integral type used for a bit-wise
 * representation of metadata about this node. The first three bits encode
 * whether or not this is a leaf node, whether or not we should default to the
 * more distant child in case of missing values, and whether or not this node
 * is categorical. The remaining bits are used to encode the feature index for
 * this node. Thus, uint8_t may be used for 2**(8 - 3) = 32 or fewer features,
 * uint16_t for 2**(16 - 3) = 8192 or fewer, and uint32_t for 536870912 or
 * fewer features.
 *
 * @tparam offset_t An integral type used to indicate the offset from
 * this node to its most distant child. This type must be large enough to store
 * the largest such offset in the forest model.
 */
template <kayak::tree_layout layout_v, typename threshold_t, typename index_t, typename metadata_storage_t, typename offset_t>
struct alignas(detail::get_node_alignment<threshold_t, index_t, metadata_storage_t, offset_t>()) node {
  /// @brief An alias for layout_v
  auto constexpr static const layout = layout_v;
  /// @brief An alias for threshold_t
  using threshold_type = threshold_t;
  /// @brief An alias for index_t
  using index_type = index_t;
  /** @brief A union to hold either a threshold value or index
   *
   * All nodes will need EITHER a threshold value, an output value, OR an index
   * to data elsewhere that wil be used either for evaluating the node (e.g. an
   * index to a categorical set) or creating an output (e.g. an index to vector
   * leaf output). This union allows us to store either of these values without
   * using additional space for the unused value.
   */
  union value_type {
    threshold_t value;
    index_t index;
  };
  /// @brief An alias for metadata_storage_t
  using metadata_storage_type = metadata_storage_t;
  /// @brief An alias for offset_t
  using offset_type = offset_t;

  // TODO(wphicks): Add custom type to ensure given child offset is at least
  // one
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnarrowing"
  HOST DEVICE constexpr node(
    threshold_type value = threshold_type{},
    bool is_leaf_node = true,
    bool default_to_distant_child = false,
    bool is_categorical_node = false,
    metadata_storage_type feature = metadata_storage_type{},
    offset_type distant_child_offset = offset_type{1}
  ) : stored_value{.value=value},
    distant_offset_minus_one{distant_child_offset - offset_type{1}},
    metadata{construct_metadata(
      is_leaf_node, default_to_distant_child, is_categorical_node, feature
    )} {}
#pragma GCC diagnostic pop

  HOST DEVICE constexpr node(
    index_type index,
    bool is_leaf_node = true,
    bool default_to_distant_child = false,
    bool is_categorical_node = false,
    metadata_storage_type feature = metadata_storage_type{},
    offset_type distant_child_offset = offset_type{1}
  ) : stored_value{.index=index},
    distant_offset_minus_one{distant_child_offset - offset_type{1}},
    metadata{construct_metadata(
      is_leaf_node, default_to_distant_child, is_categorical_node, feature
    )} {}

  /** The index of the feature for this node */
  HOST DEVICE auto constexpr feature_index() const {
    return metadata & FEATURE_MASK;
  }
  /** Whether or not this node is a leaf node */
  HOST DEVICE auto constexpr is_leaf() const {
    return bool(metadata & LEAF_MASK);
  }
  /** Whether or not to default to distant child in case of missing values */
  HOST DEVICE auto constexpr default_distant() const {
    return bool(metadata & DEFAULT_DISTANT_MASK);
  }
  /** Whether or not this node is a categorical node */
  HOST DEVICE auto constexpr is_categorical() const {
    return bool(metadata & CATEGORICAL_MASK);
  }
  /** The offset to the child of this node if it evaluates to given condition */
  HOST DEVICE auto constexpr child_offset(bool condition) const {
    if constexpr (layout == kayak::tree_layout::depth_first) {
      return offset_type{1} + condition * distant_offset_minus_one;
    } else if constexpr (layout == kayak::tree_layout::breadth_first) {
      return condition * offset_type{1} + distant_offset_minus_one;
    } else {
      static_assert(layout == kayak::tree_layout::depth_first);
    }
  }
  /** The threshold value for this node */
  HOST DEVICE auto constexpr threshold() const {
    return stored_value.value;
  }
  /** The output value for this node
   *
   * @tparam output_t The expected output type for this node.
   */
  template <bool has_vector_leaves>
  HOST DEVICE auto constexpr output() const {
    if constexpr (has_vector_leaves) {
      return stored_value.index;
    } else {
      return stored_value.value;
    }
  }

 private:
  /* Define all bit masks required to extract information from the stored
   * metadata. The first bit tells us whether or not this is a leaf node, the
   * second tells us whether or not we should default to the distant child in
   * the case of a missing value, and the third tells us whether or not this is
   * a categorical node. The remaining bits indicate the index of the feature
   * for this node */
  auto constexpr static const LEAF_BIT = metadata_storage_type(
    sizeof(metadata_storage_type) * 8 - 1
  );
  auto constexpr static const LEAF_MASK = metadata_storage_type(1 << LEAF_BIT);
  auto constexpr static const DEFAULT_DISTANT_BIT = metadata_storage_type(LEAF_BIT - 1);
  auto constexpr static const DEFAULT_DISTANT_MASK = metadata_storage_type(1 << DEFAULT_DISTANT_BIT);
  auto constexpr static const CATEGORICAL_BIT = metadata_storage_type(DEFAULT_DISTANT_BIT - 1);
  auto constexpr static const CATEGORICAL_MASK = metadata_storage_type(1 << CATEGORICAL_BIT);
  auto constexpr static const FEATURE_MASK = metadata_storage_type(
    ~(LEAF_MASK | DEFAULT_DISTANT_MASK | CATEGORICAL_MASK)
  );

  // Helper function for bit packing with the above masks
  auto static constexpr construct_metadata(
    bool is_leaf_node = true,
    bool default_to_distant_child = false,
    bool is_categorical_node = false,
    metadata_storage_type feature = metadata_storage_type{}
  ) {
    return metadata_storage_type(
      (is_leaf_node << LEAF_BIT) +
      (default_to_distant_child << DEFAULT_DISTANT_BIT) +
      (is_categorical_node << CATEGORICAL_BIT) +
      (feature & FEATURE_MASK)
    );
  }

  value_type stored_value;
  // TODO (wphicks): It may be possible to store both of the following together
  // to save bytes
  offset_type distant_offset_minus_one;
  metadata_storage_type metadata;
};

}
