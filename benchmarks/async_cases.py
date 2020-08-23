import taichi as ti
import os
import sys

sys.path.append(os.path.join(ti.core.get_repo_dir(), 'tests', 'python'))

from fuse_test_template import template_fuse_dense_x2y2z, \
    template_fuse_reduction


# Note: this is a short-term solution. In the long run we need to think about how to reuse pytest
def benchmark_async(func):
    def body():
        for arch in [ti.cpu, ti.cuda]:
            for async_mode in [True, False]:
                ti.init(arch=arch, async_mode=async_mode)
                func()
    return body

@benchmark_async
def async_benchmark_fuse_dense_x2y2z():
    template_fuse_dense_x2y2z(size=10 * 1024**2,
                              repeat=10,
                              benchmark_repeat=50,
                              benchmark=True)


@benchmark_async
def async_benchmark_fuse_reduction():
    template_fuse_reduction(size=10 * 1024**2,
                            repeat=10,
                            benchmark_repeat=50,
                            benchmark=True)