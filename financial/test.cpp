#include "portfolio.h"

int main(int argc, char **argv)
{
  Finance::Account a, b, c, d;
  Finance::Portfolio p;

  p.add_account(&a);
  p.add_account(&b);
  p.add_account(&c);
  p.add_account(&d);

  return 0;
}
