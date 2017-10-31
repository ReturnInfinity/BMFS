#!/bin/bash

set -e
set -u

travis-ci/do-make-build.bash
travis-ci/do-cmake-build.bash

