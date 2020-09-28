#!/usr/bin/env bash
set -e

for python_path in /opt/python/cp{27,35,36,37}*m; do
    python=$python_path/bin/python
    pip=$python_path/bin/pip

    cd /asapo/build
    cmake -DENABLE_LIBFABRIC=on -DCMAKE_BUILD_TYPE="Release" -DLIBCURL_DIR=/curl -DPython_EXECUTABLE=$python ..
    cd consumer \
        && $pip install -r /asapo/consumer/api/python/dev-requirements.txt \
        && make \
        && $pip wheel api/python/source_dist_linux/dist/*.tar.gz -w wheelhouse --no-deps
    cd ../producer \
        && $pip install -r /asapo/producer/api/python/dev-requirements.txt \
        && make \
        && $pip wheel api/python/source_dist_linux/dist/*.tar.gz -w wheelhouse --no-deps
done

cd ../consumer \
    && for wheel in wheelhouse/asapo_consumer*.whl; do
        auditwheel repair $wheel --plat manylinux2010_x86_64 -w /asapo/build/wheelhouse
    done

cd ../producer \
    && for wheel in wheelhouse/asapo_producer*.whl; do
        auditwheel repair $wheel --plat manylinux2010_x86_64 -w /asapo/build/wheelhouse
    done