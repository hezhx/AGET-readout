# AGET-readout
## 概述
[主文件](./Data_Analysis.C)为Data_Analysis.C。
Data_Analysis_Draw.C为不成熟的程序。
Data_Analysis把二进制的数据[源代码](./test.dat)写入一个`card_info`三维数组。

```
root -l
[0] .L Data_Analysis.C
[1] Analysis("test.dat")
```

`card_info`中关于board (card), chip, channel和信号的信息被[txt2root](./txt2root.C)写入root文件。
