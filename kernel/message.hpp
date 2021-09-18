#pragma once
#include <cstdint>

enum class LayerOperation {
  Move, MoveRelative, Draw, DrawArea
};

struct Message {
  enum Type {
    kInterruptXHCI,
    kTimerTimeout,
    kKeyPush,
    kDefault,
  } type;

  void* src_task;

  union {
    struct {
      unsigned long timeout;
      int value;
    } timer;

    struct {
      uint8_t modifier;
      uint8_t keycode;
      char ascii;
    } keyboard;

    struct {
      LayerOperation op;
      unsigned int layer_id;
      int x, y;
      int w, h;
    } layer;
  } arg;

  Message(Message::Type type = Message::kDefault): type{type}{}

  Message(const Message& m): type{m.type}, src_task{m.src_task}, arg{m.arg} {}
};

