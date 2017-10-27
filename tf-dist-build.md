# tensorflow 静态文件打包的流程

本文详细给出将tensorflow静态文件打包，并用于外部C++项目的过程。以tensorflow 1.3.0为例。

- 1 下载源代码：

```
git clone --recurse-submodules -b v1.3.0 https://github.com/tensorflow/tensorflow.git
cd tensorflow
```

- 2 配置参数：

```bash
./configure
```

- 3 构建c++共享库

```
bazel build -c opt --verbose_failures //tensorflow:libtensorflow_cc.so
```
- 4 编译protobuf依赖

这个过程需要安装automake,libtool,autoconf以及cmake，在osx系统中安装命令如下：

```bash
brew install autoconf libtool, automake, cmake
```

安装完成之后，运行如下命令：


```
mkdir /tmp/proto
tensorflow/contrib/makefile/download_dependencies.sh
cd tensorflow/contrib/makefile/downloads/protobuf/
./autogen.sh
./configure --prefix=/tmp/proto/
make
make install
```

- 5 编译eigen依赖

```
mkdir /tmp/eigen
cd ../eigen
mkdir build_dir
cd build_dir
cmake -DCMAKE_INSTALL_PREFIX=/tmp/eigen/ ../
make install
```

- 6 复制所需文件到项目目录下面（以本项目为例，为tf-dist目录）

回到tensorflow source目录：


```
sudo cp -r tensorflow segment/tf-dist/
sudo find segment/tf-dist/tensorflow -type f  ! -name "*.h" -delete

sudo cp bazel-genfiles/tensorflow/core/framework/*.h  segment//tf-dist/tensorflow/core/framework
sudo cp bazel-genfiles/tensorflow/core/lib/core/*.h  segment//tf-dist/tensorflow/core/lib/core
sudo cp bazel-genfiles/tensorflow/core/protobuf/*.h  segment//tf-dist/tensorflow/core/protobuf
sudo cp bazel-genfiles/tensorflow/core/util/*.h  segment//tf-dist/tensorflow/core/util
sudo cp bazel-genfiles/tensorflow/cc/ops/*.h  segment//tf-dist/tensorflow/cc/ops

sudo cp -r third_party segment/tf-dist/
sudo rm -r segment/tf-dist/third_party/py

sudo cp bazel-bin/tensorflow/libtensorflow_cc.so segment/tf-dist/lib/
sudo chmod 777 segment/tf-dist/lib/libtensorflow_cc.so 

sudo cp -r /tmp/proto/include/google segment/tf-dist
sudo cp -r /tmp/eigen/include/eigen3/Eigen segment/tf-dist
sudo cp -r /tmp/eigen/include/eigen3/unsupported segment/tf-dist

```

- 7 配置项目文件

在项目的WORKSPACE加入下列配置：

```
new_local_repository(
   name="tf",
   path = "tf-dist",
   build_file="tf-dist/BUILD",
)
```

其中tf-dist/BUILD文件的内容为：

```
# Bazel build file for binary tf
licenses(["notice"])

cc_library(
  name="tensorflow",
  hdrs = glob(["tensorflow/*", "google/*"]),
  includes = [
    ".",
  ],
  visibility = ["//visibility:public"],
  srcs=["lib/libtensorflow_cc.so",],
)
```
