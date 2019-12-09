# chatroom

A console based server/client chatroom using Sockets and NCurses windows.

![Application Image](chatroom.png)

## Building

Prerequisites
- GCC
- Clang
- Make
- NCurses

```bash
sudo apt-get update && sudo apt-get install clang-5.0 libc++abi-dev libc++-dev git gdb valgrind graphviz imagemagick gnuplot
sudo apt-get install libncurses5-dev libncursesw5-dev
git clone https://github.com/realeigenvalue/chatroom.git
cd chatroom
make
./server <port>
./chatroom <address> <port> <username>
```

## Contributing
Pull requests are welcome. For major changes, please open an issue first to discuss what you would like to change.

Please make sure to update tests as appropriate.

## License
GNU GPLv3
