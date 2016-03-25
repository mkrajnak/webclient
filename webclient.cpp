#include <iostream>

using namespace std;

int main(int argc, char **argv)
{

  cout << "Hello World!" << endl;

  if (argc >= 1) {
      cout << argv[1] << endl;
  }

  return 0;
}
