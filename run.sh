#!/bin/bash

# 編譯C程式
gcc simple_filter.c -o simple_filter -lm

# 檢查是否編譯成功
if [ $? -eq 0 ]; then
    echo "編譯成功。"

    # 執行三個不同的指令
    ./simple_filter 1024 hL1024.txt hR1024.txt YL1024.txt YR1024.txt blue_giant_fragment.wav output1024.wav
    ./simple_filter 32 hL32.txt hR32.txt YL32.txt YR32.txt blue_giant_fragment.wav output32.wav
    ./simple_filter 8 hL8.txt hR8.txt YL8.txt YR8.txt blue_giant_fragment.wav output8.wav
    
    # 執行 Python 腳本
    python show_data.py

else
    echo "錯誤：編譯失敗。"
fi
