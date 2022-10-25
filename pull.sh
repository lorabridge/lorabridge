#!/bin/bash

git pull --recurse-submodules
git submodule foreach 'git checkout main'
git submodule foreach 'git pull'
