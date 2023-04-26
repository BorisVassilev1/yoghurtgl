#!/bin/bash

find . -name '*.cpp' -o -name '*.h' | entr -csdr 'cmake --build ./build --target=YoghurtGL'
