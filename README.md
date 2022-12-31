# GameboyC
A C-like compiler for the GameBoy. 

## How does it work?
It produces .asm files that can be compiled into .gb files using rgbds (https://rgbds.gbdev.io/). It uses SFML to read .png files, but the translator itself is fully independant.

> .\gc.exe file_to_compile.gc

## Language
The compiler tries to be as close as possible to C. 
There is only one variable type: a 8-bit value.
```
int value = 0;
```
All functions return an integer, although return isn't needed to be specified.
```
int main()
{
  //...
}
```
If and while are the only control structures available.
```
if(...)
{
  //...
}

while(...)
{
  //...
}
```
Operators are:
Boolean expressions: `==, !=, >, <, &, !, |, ^`
Expressions: `*, +, -, /`

`true` evaluates to 0xff
`false` evalutes to 0
