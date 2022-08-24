#pragma once
#include <cstddef>
#include <stddef.h>
#include <herring3/detail/evaluate_tree.hpp>
#include <herring3/detail/gpu_introspection.hpp>
#include <herring3/detail/postprocessor.hpp>
#include <herring3/detail/infer_kernel/shared_memory_buffer.cuh>
#include <kayak/ceildiv.hpp>
#include <kayak/padding.hpp>

namespace herring {
namespace detail {

template<bool is_categorical, typename forest_t,
  typename vector_output_t=std::nullptr_t>
__global__ void infer_kernel(
    forest_t forest,
    postprocessor<
      typename forest_t::leaf_output_type, typename forest_t::io_type
    > postproc,
    typename forest_t::io_type* output,
    typename forest_t::io_type const* input,
    size_t row_count,
    size_t col_count,
    size_t num_outputs,
    size_t shared_mem_byte_size,
    size_t output_workspace_size,
    size_t chunk_size,
    vector_output_t vector_output_p=nullptr
) {
  auto constexpr has_vector_leaves = !std::is_same_v<
    vector_output_t, std::nullptr_t
  >;
  extern __shared__ std::byte shared_mem_raw[];

  auto shared_mem = shared_memory_buffer(shared_mem_raw, shared_mem_byte_size);

  using node_t = typename forest_t::node_type;

  using output_t = std::conditional_t<
    has_vector_leaves,
    vector_output_t,
    typename node_t::threshold_type
  >;

  using io_t = typename forest_t::io_type;

  for (
    auto i=blockIdx.x * chunk_size;
    i < row_count;
    i += chunk_size * gridDim.x
  ) {

    shared_mem.clear();
    auto* output_workspace = shared_mem.fill<output_t>(output_workspace_size);

    // Handle as many rows as requested per loop or as many rows as are left to
    // process
    auto rows_in_this_iteration = min(chunk_size, row_count - i);

    auto* input_data = shared_mem.copy(
      input + i * col_count,
      rows_in_this_iteration,
      col_count
    );

    auto task_count = rows_in_this_iteration * forest.tree_count();

    auto num_grove = kayak::ceildiv(
      min(size_t{blockDim.x}, task_count),
      rows_in_this_iteration
    );

    // Note that this sync is safe because every thread in the block will agree
    // on whether or not a sync is required
    shared_mem.sync();

    // Every thread must iterate the same number of times in order to avoid a
    // deadlock on __syncthreads, so we round the task_count up to the next
    // multiple of the number of threads in this block. We then only perform
    // work within the loop if the task_index is below the actual task_count.
    auto const task_count_rounded_up = blockDim.x * kayak::ceildiv(task_count, blockDim.x);

    // Infer on each tree and row
    for (
      auto task_index = threadIdx.x;
      task_index < task_count_rounded_up;
      task_index += blockDim.x
    ) {
      auto real_task = task_index < task_count;
      auto row_index = task_index * real_task % rows_in_this_iteration;
      auto tree_index = task_index * real_task / rows_in_this_iteration;
      auto grove_index = threadIdx.x / rows_in_this_iteration;

      if constexpr (has_vector_leaves) {
        auto leaf_index = evaluate_tree<typename
          forest_t::leaf_output_type
        >(
          forest.get_tree_root(tree_index),
          input_data + row_index * col_count
        );
        for (
          auto class_index=size_t{};
          class_index < num_outputs;
          ++class_index
        ) {
          output_workspace[
            row_index * num_outputs * num_grove
            + class_index * num_grove
            + grove_index
          ] += vector_output_p[
            leaf_index * num_outputs + class_index
          ];
        }
      } else {
        auto output_offset = (
          row_index * num_outputs * num_grove
          + (tree_index % num_outputs) * num_grove
          + grove_index
        ) * (task_index < task_count);
        output_workspace[output_offset] += evaluate_tree<
          typename forest_t::leaf_output_type
        >(
          forest.get_tree_root(tree_index),
          input_data + row_index * col_count
        ) * real_task;
      }
      __syncthreads();
    }

    auto padded_num_groves = kayak::padded_size(num_grove, WARP_SIZE);
    for (
      auto row_index = threadIdx.x / WARP_SIZE;
      row_index < rows_in_this_iteration;
      row_index += blockDim.x / WARP_SIZE
    ) {
      for (
        auto class_index = size_t{};
        class_index < num_outputs;
        ++class_index
      ) {
        auto grove_offset = (
          row_index * num_outputs * num_grove + class_index * num_grove
        );
        auto class_sum = output_t{};
        for (
          auto grove_index = threadIdx.x % WARP_SIZE;
          grove_index < padded_num_groves;
          grove_index += WARP_SIZE
        ) {
          auto real_thread = grove_index < num_grove;
          auto out_index = grove_offset + grove_index * real_thread;
          class_sum *= (threadIdx.x % WARP_SIZE == 0);
          class_sum += output_workspace[out_index] * real_thread;
          for (
            auto thread_offset = (WARP_SIZE >> 1); 
            thread_offset > 0;
            thread_offset >>= 1
          ) {
            class_sum += __shfl_down_sync(
              0xFFFFFFFF,
              class_sum,
              thread_offset
            );
          }
        }
        if (threadIdx.x % WARP_SIZE == 0) {
          output_workspace[grove_offset] = class_sum;
        }
      }
      if (threadIdx.x % WARP_SIZE == 0) {
        postproc(
          output_workspace + row_index * num_outputs * num_grove,
          num_outputs, 
          output + ((i + row_index) * num_outputs)
        );
      }
    }
    __syncthreads();
  }
}

}
}