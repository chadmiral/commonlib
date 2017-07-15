#pragma once

// http://gameprogrammingpatterns.com/bytecode.html

namespace Game
{
  class VirtualMachine
  {
    enum VMOperation
    {
      VM_OP_ADD,
      VM_OP_SUB,
      VM_OP_MUL,
      VM_OP_SUB,

      NUM_VM_OPS
    };

  class VMExpressionNode
  {
  public:
    VMExpressionNode() {}
    ~VMExpressionNode() {}
  private:
    VMOperation         _op;
    VMExpressionNode   *_a;
    VMExpressionNode   *_b;
  };

  class VMExpressionTree
  {
  public:
  private:
  };

  public:
    VirtualMachine() {}
    ~VirtualMachine() {}

    VMExpressionTree *parse_expression(std::string)
    {
      char *c = exp;
      while(*c != '\0')
      {

      }
    }

    float evaluate(char *instruction, uint8_t instruction_length)
    {
      // 1 byte each
      //----------------------------------------------------+
      //|  OP CODE  |  operand  A  | operand B |  operand C |
      //+---------------------------------------------------+

      uint8_t stack[VM_MAX_STACK_SIZE];

      uint8_t op_code = instruction << 6;
      uint8_t operand_a = instruction << 4;
      uint8_t operand_b = instruction ^ 0xffffff00;

      switch(op_code)
      {
        case VM_OP_ADD:

          break;
        case VM_OP_SUB:
          break;
        case VM_OP_MUL:
          break;
        case VM_OP_SUB:
          break;
        default:
          break;
      }
    }
  private:
  };
};
