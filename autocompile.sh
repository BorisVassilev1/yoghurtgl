#!/bin/bash

find . -name '*.cpp' -o -name '*.h' | entr -csdr 'cd ./build && make YoghurtGL'
