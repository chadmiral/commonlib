#pragma once

#include <SDL.h>
#include <SDL_Thread.h>

class ReaderWriterLock
{
private:
  /*
  SDL_mutex *_reader_lock;
  SDL_mutex *_global_lock;
  int        _num_blockers;
  */
  SDL_mutex *_lock;
  SDL_cond  *_turn;
  
  int        _num_writers;
  int        _num_writing;
  int        _num_reading;
public:
  ReaderWriterLock()
  {
    /*
    _num_blockers = 0;
    _reader_lock = SDL_CreateMutex();
    _global_lock = SDL_CreateMutex();
    */
    _lock = SDL_CreateMutex();
    _turn = SDL_CreateCond();
    _num_reading = 0;
    _num_writing = 0;
    _num_writers = 0;
  }
  ~ReaderWriterLock()
  {
    /*
    SDL_DestroyMutex(_reader_lock);
    SDL_DestroyMutex(_global_lock);
    */
    SDL_DestroyMutex(_lock);
    SDL_DestroyCond(_turn);
  }

  void reader_lock()
  {
    /*
    SDL_LockMutex(_reader_lock);
    _num_blockers++;
    if (_num_blockers == 1)
    {
      SDL_LockMutex(_global_lock);
    }
    SDL_UnlockMutex(_reader_lock);
    */
    SDL_LockMutex(_lock);
    while (_num_writers)
    {
      SDL_CondWait(_turn, _lock);
    }
    _num_reading++;
    SDL_UnlockMutex(_lock);
  }

  void reader_unlock()
  {
    /*
    SDL_LockMutex(_reader_lock);
    _num_blockers--;
    if (_num_blockers == 0)
    {
      SDL_UnlockMutex(_global_lock);
    }
    SDL_UnlockMutex(_reader_lock);
    */
    SDL_LockMutex(_lock);
    _num_reading--;
    SDL_CondBroadcast(_turn);
    SDL_UnlockMutex(_lock);
  }

  void writer_lock() 
  {
    //SDL_LockMutex(_global_lock);
    SDL_LockMutex(_lock);
    _num_writers++;
    while (_num_reading || _num_writing)
    {
      SDL_CondWait(_turn, _lock);
    }
    _num_writing++;
    SDL_UnlockMutex(_lock);
  }
  void writer_unlock()
  {
    //SDL_UnlockMutex(_global_lock);

    SDL_LockMutex(_lock);
    _num_writing--;
    _num_writers--;
    SDL_CondBroadcast(_turn);
    SDL_UnlockMutex(_lock);
  }
};
