#pragma once
#include <halp/controls.hpp>
#include <halp/controls.enums.hpp>
#include <halp/messages.hpp>
#include <halp/meta.hpp>

#include <algorithm>
#include <cctype>
#include <string>

// Framework-agnostic halp object (no ossia/Qt): transforms an input string. Compiles
// unchanged to the ossia/score back-end and, standalone, to Max/Pd/Python/TD/Godot.
class DemoProcessor
{
public:
  halp_meta(name, "Avnd Addon Demo")
  halp_meta(c_name, "avnd_addon_demo")
  halp_meta(category, "Demo")
  halp_meta(author, "Avendish")
  halp_meta(description, "Transform an input string (case / reverse, optional prefix)")
  halp_meta(uuid, "0b6a7c1e-9d24-4e3a-8f15-2c7b4a90d6e1")

  enum class op
  {
    uppercase,
    lowercase,
    reverse
  };

  struct ins
  {
    struct : halp::val_port<"Input", std::string>
    {
      void update(DemoProcessor& self) { self(); }
    } text;
    halp::enum_t<op, "Operation"> operation;
  } inputs;

  struct
  {
    halp::val_port<"Output", std::string> result;
  } outputs;

  void operator()();

  void set_prefix(std::string p) { prefix = std::move(p); }
  halp_start_messages(DemoProcessor)
    halp_mem_fun(set_prefix)
  halp_end_messages

private:
  std::string prefix;
};

inline void DemoProcessor::operator()()
{
  std::string out = inputs.text.value;
  switch(inputs.operation.value)
  {
    case op::uppercase:
      std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c) {
        return std::toupper(c);
      });
      break;
    case op::lowercase:
      std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c) {
        return std::tolower(c);
      });
      break;
    case op::reverse:
      std::reverse(out.begin(), out.end());
      break;
  }
  outputs.result.value = prefix + out;
}
