# FensterB

[Fenster](https://github.com/zserge/fenster) with some bloated code

## License

Code is distributed under MIT license, feel free to use it in your proprietary projects as well.

### Main changes
- Source split by system
- Platform-agnostic structure for audio
- Mouse: Left, Right, Middle, Scrollup/down

### Mouse buttons
Inside fenster object:
```C
  int mclick[5];  // left, right, middle, scroll up, scroll down (cleared after loop)
  int mhold[3];   // left, right, middle (persistent until release)
  //0 = false, 1 = true
```
