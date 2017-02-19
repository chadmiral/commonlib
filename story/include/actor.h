#ifndef __ACTOR_H__
#define __ACTOR_H__

class Interaction
{
public:
  Interaction(Actor *_a, Actor *);
  ~Interaction();
private:
  Actor *subject, *object;
  std::string pre, mid, post;
};

class Actor
{
public:
  Actor() {}
  ~Actor() {}
private:
  std::string name;
};

#endif //__ACTOR_H__
