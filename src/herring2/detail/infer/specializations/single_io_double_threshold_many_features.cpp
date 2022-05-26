#include <herring2/detail/infer/cpu_predict.cuh>
#include <kayak/device_type.hpp>
#include <kayak/tree.hpp>

namespace herring {
namespace detail {
namespace inference {

// single IO, double threshold, many features
template void predict<kayak::device_type::cpu, forest<kayak::tree_layout::depth_first, double, uint32_t, uint16_t, uint32_t, float, false>, float>(forest<kayak::tree_layout::depth_first, double, uint32_t, uint16_t, uint32_t, float, false> const&, kayak::data_array<kayak::data_layout::dense_row_major, float>&, kayak::data_array<kayak::data_layout::dense_row_major, float> const&, kayak::raw_index_t, element_op, row_op, float, float, float, int, kayak::cuda_stream);
template void predict<kayak::device_type::cpu, forest<kayak::tree_layout::depth_first, double, uint32_t, uint16_t, uint32_t, float, true>, float>(forest<kayak::tree_layout::depth_first, double, uint32_t, uint16_t, uint32_t, float, true> const&, kayak::data_array<kayak::data_layout::dense_row_major, float>&, kayak::data_array<kayak::data_layout::dense_row_major, float> const&, kayak::raw_index_t, element_op, row_op, float, float, float, int, kayak::cuda_stream);
template void predict<kayak::device_type::cpu, forest<kayak::tree_layout::depth_first, double, uint32_t, uint16_t, uint32_t, double, false>, float>(forest<kayak::tree_layout::depth_first, double, uint32_t, uint16_t, uint32_t, double, false> const&, kayak::data_array<kayak::data_layout::dense_row_major, float>&, kayak::data_array<kayak::data_layout::dense_row_major, float> const&, kayak::raw_index_t, element_op, row_op, float, float, float, int, kayak::cuda_stream);
template void predict<kayak::device_type::cpu, forest<kayak::tree_layout::depth_first, double, uint32_t, uint16_t, uint32_t, double, true>, float>(forest<kayak::tree_layout::depth_first, double, uint32_t, uint16_t, uint32_t, double, true> const&, kayak::data_array<kayak::data_layout::dense_row_major, float>&, kayak::data_array<kayak::data_layout::dense_row_major, float> const&, kayak::raw_index_t, element_op, row_op, float, float, float, int, kayak::cuda_stream);
template void predict<kayak::device_type::cpu, forest<kayak::tree_layout::depth_first, double, uint32_t, uint16_t, uint32_t, uint32_t, false>, float>(forest<kayak::tree_layout::depth_first, double, uint32_t, uint16_t, uint32_t, uint32_t, false> const&, kayak::data_array<kayak::data_layout::dense_row_major, float>&, kayak::data_array<kayak::data_layout::dense_row_major, float> const&, kayak::raw_index_t, element_op, row_op, float, float, float, int, kayak::cuda_stream);
template void predict<kayak::device_type::cpu, forest<kayak::tree_layout::depth_first, double, uint32_t, uint16_t, uint32_t, uint32_t, true>, float>(forest<kayak::tree_layout::depth_first, double, uint32_t, uint16_t, uint32_t, uint32_t, true> const&, kayak::data_array<kayak::data_layout::dense_row_major, float>&, kayak::data_array<kayak::data_layout::dense_row_major, float> const&, kayak::raw_index_t, element_op, row_op, float, float, float, int, kayak::cuda_stream);
template void predict<kayak::device_type::cpu, forest<kayak::tree_layout::depth_first, double, uint32_t, uint32_t, uint32_t, float, false>, float>(forest<kayak::tree_layout::depth_first, double, uint32_t, uint32_t, uint32_t, float, false> const&, kayak::data_array<kayak::data_layout::dense_row_major, float>&, kayak::data_array<kayak::data_layout::dense_row_major, float> const&, kayak::raw_index_t, element_op, row_op, float, float, float, int, kayak::cuda_stream);
template void predict<kayak::device_type::cpu, forest<kayak::tree_layout::depth_first, double, uint32_t, uint32_t, uint32_t, float, true>, float>(forest<kayak::tree_layout::depth_first, double, uint32_t, uint32_t, uint32_t, float, true> const&, kayak::data_array<kayak::data_layout::dense_row_major, float>&, kayak::data_array<kayak::data_layout::dense_row_major, float> const&, kayak::raw_index_t, element_op, row_op, float, float, float, int, kayak::cuda_stream);
template void predict<kayak::device_type::cpu, forest<kayak::tree_layout::depth_first, double, uint32_t, uint32_t, uint32_t, double, false>, float>(forest<kayak::tree_layout::depth_first, double, uint32_t, uint32_t, uint32_t, double, false> const&, kayak::data_array<kayak::data_layout::dense_row_major, float>&, kayak::data_array<kayak::data_layout::dense_row_major, float> const&, kayak::raw_index_t, element_op, row_op, float, float, float, int, kayak::cuda_stream);
template void predict<kayak::device_type::cpu, forest<kayak::tree_layout::depth_first, double, uint32_t, uint32_t, uint32_t, double, true>, float>(forest<kayak::tree_layout::depth_first, double, uint32_t, uint32_t, uint32_t, double, true> const&, kayak::data_array<kayak::data_layout::dense_row_major, float>&, kayak::data_array<kayak::data_layout::dense_row_major, float> const&, kayak::raw_index_t, element_op, row_op, float, float, float, int, kayak::cuda_stream);
template void predict<kayak::device_type::cpu, forest<kayak::tree_layout::depth_first, double, uint32_t, uint32_t, uint32_t, uint32_t, false>, float>(forest<kayak::tree_layout::depth_first, double, uint32_t, uint32_t, uint32_t, uint32_t, false> const&, kayak::data_array<kayak::data_layout::dense_row_major, float>&, kayak::data_array<kayak::data_layout::dense_row_major, float> const&, kayak::raw_index_t, element_op, row_op, float, float, float, int, kayak::cuda_stream);
template void predict<kayak::device_type::cpu, forest<kayak::tree_layout::depth_first, double, uint32_t, uint32_t, uint32_t, uint32_t, true>, float>(forest<kayak::tree_layout::depth_first, double, uint32_t, uint32_t, uint32_t, uint32_t, true> const&, kayak::data_array<kayak::data_layout::dense_row_major, float>&, kayak::data_array<kayak::data_layout::dense_row_major, float> const&, kayak::raw_index_t, element_op, row_op, float, float, float, int, kayak::cuda_stream);

}
}
}
