#!/usr/bin/env bash
set -e

declare -A numpy_versions
numpy_versions[cp27mu]=1.12.1
numpy_versions[cp27m]=1.12.1
numpy_versions[cp35m]=1.12.1
numpy_versions[cp36m]=1.12.1
numpy_versions[cp37m]=1.14.5
numpy_versions[cp38]=1.17.3

for python_path in /opt/python/cp{27,35,36,37,38}*; do
    python_version=$(basename $python_path)
    python_version=${python_version#*-}
    python=$python_path/bin/python
    pip=$python_path/bin/pip
    numpy_version=${numpy_versions[$python_version]}
    echo "building wheel for python_version=$python_version with numpy_version=$numpy_version"

    cd /asapo/build
    cmake -DENABLE_LIBFABRIC=on -DCMAKE_BUILD_TYPE="Release" -DLIBCURL_DIR=/curl -DPython_EXECUTABLE=$python -DNUMPY_VERSION=$numpy_version ..
    cd /asapo/build/consumer/api/python/source_dist_linux \
        && $pip install -r ../dev-requirements.txt \
        && make python-dist \
        && $pip wheel dist/*.tar.gz -w /asapo/build/consumer/wheelhouse --no-deps
    cd  /asapo/build/producer/api/python/source_dist_linux \
        && $pip install -r ../dev-requirements.txt \
        && make python-dist-producer \
        && $pip wheel dist/*.tar.gz -w /asapo/build/producer/wheelhouse --no-deps
done

cd /asapo/build/consumer \
    && for wheel in wheelhouse/asapo_consumer*.whl; do
        auditwheel repair $wheel --plat manylinux2010_x86_64 -w /asapo/build/wheelhouse
    done

cd /asapo/build/producer \
    && for wheel in wheelhouse/asapo_producer*.whl; do
        auditwheel repair $wheel --plat manylinux2010_x86_64 -w /asapo/build/wheelhouse
    done