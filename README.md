# CS2610: Computer Organization and Architecture Scalar Pipelined Processor Design

## Rohan Kadam(CS22B083)


## Run and Executing the code


  1) Save the contents using the input files as follows

```bash
   a. Instruction cache in ICache.txt
   b. Memory data in DCache.txt.
   c. Register file in RF.txt
```

  2) To run the code use following commands : 
```bash
   a. g++ main.cpp
   b. ./a.out
```
  3) The output is stored in Output.txt and DCache.txt as follows :  
```bash
  a. DCache.txt : contents of Data Cache reflecting all changes applied during execution.
  b. Output.txt : contains the stats described below : 

Assuming that each pipeline stage takes 1 cycle, execute a given program 
1. Number of instructions executed.
2. Number of instructions of each type.
3. CPI (clock cycles per instruction)
4. Number of stalls and reason for each stall (eg: RAW dependency).
```
