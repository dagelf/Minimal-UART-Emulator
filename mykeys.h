#include <windows.h>
#include <thread>
#include <atomic>

#define kbuf 1000           // how many keys can you press in 10ms? (Or however long you think your main thread will take... 
char key[kbuf],keyo=0;      // Keyboard repeat rate limits this... but input can be redirected, exceeding the repeat rate of just a single keyboard
std::atomic<char> keyi;     // Convenient... but I can't help but wonder about the implementation & performance impact.
                            // not that it's relevant here... but it could be on a battery sensitive IoT platform

bool key_avail() {
    return keyo<keyi;
}

void key_pop(char &c) {
    c=key[keyo++];
    if (keyi==keyo) {keyi=0;keyo=0;}
}

char key_pop() {
  char c;
  c=key[keyo++];
  if (keyi==keyo) {keyi=0;keyo=0;}
  return c;
}

void key_thread() {
 HANDLE hStdInput;
 INPUT_RECORD ir[kbuf];
 DWORD nRead;
// COORD xy;
 UINT i;

 hStdInput=GetStdHandle(STD_INPUT_HANDLE);
 FlushConsoleInputBuffer(hStdInput);
 while(1)
 {
  ReadConsoleInput(hStdInput,ir,kbuf,&nRead);
  for(i=0;i<nRead;i++)
  {
      switch(ir[i].EventType)
      {
       case KEY_EVENT:
            if (ir[i].Event.KeyEvent.bKeyDown)               //You can also pick up key releases, but we only want presses for now
            {
                if (keyi<kbuf-1) key[keyi++]=ir[i].Event.KeyEvent.uChar.AsciiChar;
//                xy.X=0;xy.Y=0; SetConsoleCursorPosition(hStdOutput,xy);
//                printf("AsciiCode = %d: symbol = %c\n",ir[i].Event.KeyEvent.uChar.AsciiChar,ir[i].Event.KeyEvent.uChar.AsciiChar);
            }
            break;
/*       case MOUSE_EVENT:
//            xy.X=0, xy.Y=1; SetConsoleCursorPosition(hStdOutput,xy);
//            printf("%.3d\t%.3d\t%.3d",ir[i].Event.MouseEvent.dwMousePosition.X,ir[i].Event.MouseEvent.dwMousePosition.Y,(int)ir[i].Event.MouseEvent.dwButtonState&0x07  //mask out scroll wheel
         );
         break;*/
      }
  }
 };
} // key_thread

void start_key_listen_thread() {
    keyi=0;
    std::thread t(key_thread);
    t.detach();
}
