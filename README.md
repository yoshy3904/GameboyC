# GameboyC
A C-like compiler for the GameBoy written in C++. Using rgbds (https://rgbds.gbdev.io/) and SFML to convert images to assembly.

## Language
8-bit integer.
```
int value = 0;
```
Functions
```
int add(int a, int b)
{
  return a + b;
}

int main()
{
  //...
}
```
Control Structures.
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
Boolean expressions: `==, !=, >, <, &, !, |, ^`
Expressions: `*, +, -, /`

`true` evaluates to 0xff
`false` evalutes to 0

Inline assembly.
```
asm{
  ld a, 10
  ...
};
```
