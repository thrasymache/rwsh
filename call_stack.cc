// The definition of the Call_stack and Conditional_state classes. The former
// encapsulates the variables that had been static within Executable classes,
// and the latter encapsulates the state used by if_core
//
// Copyright (C) 2019 Samuel Newbold

#include <algorithm>
#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "arg_spec.h"
#include "rwsh_stream.h"
#include "variable_map.h"

#include "argm.h"
#include "arg_script.h"
#include "builtin.h"
#include "call_stack.h"
#include "executable.h"
#include "executable_map.h"
#include "prototype.h"

#include "function.h"

void Call_stack::add_error(void) {
  unwind_stack_v = true;
  ++current_exception_count;}

// code to call exception handlers when requested within a function
void Call_stack::catch_blocks(const Argm& argm, Error_list& exceptions) {
  unsigned dropped_catches = 0;
  for (auto focus = exceptions.begin(); focus != exceptions.end();)
    if (find(argm.begin() + 1, argm.end(), (*focus)[0]) != argm.end()) {
      if (dropped_catches >= max_extra) {
        if (!execution_handler_excess_thrown)
          exceptions.add_error(Exception(Argm::Excessive_exceptions_in_catch,
                                         max_extra));
         execution_handler_excess_thrown = true;
         return;}
      unsigned previous_count = current_exception_count;
      in_exception_handler_v = true;
      unwind_stack_v = false;
      executable_map.run(*focus, exceptions);
      in_exception_handler_v = false;
      dropped_catches += current_exception_count - previous_count;
      if (!unwind_stack()) {
        focus = exceptions.erase(focus);
        --current_exception_count;}
      else focus++;}
    else focus++;
  if (current_exception_count || exit_requested) unwind_stack_v = true;
  else unwind_stack_v = false;
  in_exception_handler_v = false;}

// run one exception handler restoring unwind_stack afterwards
void Call_stack::catch_one(Argm& argm, Error_list& exceptions) {
  in_exception_handler_v = true;
  unwind_stack_v = false;
  executable_map.run(argm, exceptions);
  in_exception_handler_v = false;
  unwind_stack_v = true;}

int Call_stack::collect_errors_core(const Argm& argm, bool logic,
                                    Error_list& parent) {
  Argm blank(argm.parent_map(), argm.input, argm.output.child_stream(),
             argm.error);
  blank.push_back(".mapped_argfunction");
  std::vector<std::string> other(argm.begin()+1, argm.end());
  int ret;
  for (auto j: *argm.argfunction()) {
    if (current_exception_count > max_collect) {
      if (!collect_excess_thrown)
        parent.add_error(Exception(Argm::Excessive_exceptions_collected,
                                   max_collect));
      unwind_stack_v = collect_excess_thrown = true;
      return ret;}
    Error_list children;
    Argm statement_argm = j.interpret(blank, children);
    if (!global_stack.unwind_stack())
      ret = executable_map.run(statement_argm, children);
    if (children.size()) {
      unwind_stack_v = false;
      for (auto k: children) {
        parent.push_back(k);
        if (logic == (find(other.begin(), other.end(), k[0]) != other.end()))
          unwind_stack_v = true;}}} // will cause subsequent j to not run
  if (parent.size()) unwind_stack_v = true;
  return ret;}

/* code to call exception handlers, separated out of operator() for clarity.
   The requirements for stack unwinding to work properly are as
   follows: any code that runs more than one other executable must test
   unwind_stack() in between each executable, which must be of an Executable
   rather than a direct call to the function that implements a builtin (the
   code below to run the fallback_handler is the only exception to this rule).
   main does not need to do this handling, because anything that it calls
   directly will have an initial nesting of 0.*/
void Call_stack::exception_handler(Error_list& exceptions) {
  in_exception_handler_v = true;
  unwind_stack_v = false;
  std::set<std::string> failed_handlers;
  for(;exceptions.size(); exceptions.pop_front(), --current_exception_count){
    Argm& focus(exceptions.front());
    if (failed_handlers.find(focus[0]) == failed_handlers.end()) {
      unsigned previous_count = current_exception_count;
      executable_map.run(focus, exceptions);
      if (unwind_stack() || current_exception_count > previous_count) {
        failed_handlers.insert(focus[0]);
        exceptions.insert(++exceptions.begin(), exceptions.back());
        exceptions.pop_back();
        unwind_stack_v = false;}}
    if (failed_handlers.find(focus[0]) != failed_handlers.end())
      b_fallback_handler(Argm(".fallback_handler", focus.argv(),
                              focus.argfunction(), Variable_map::global_map,
                              focus.input, focus.output, focus.error),
                         exceptions);}
  collect_excess_thrown = execution_handler_excess_thrown = false;
  in_exception_handler_v = false;}

bool Call_stack::remove_exceptions(const std::string &name,
                                   Error_list& exceptions) {
  bool ret = false;
  bool unwind_stack_before = unwind_stack_v;
  for (auto focus = exceptions.begin(); focus != exceptions.end();)
    if ((*focus)[0] == name) {
      ret = true;
      focus = exceptions.erase(focus);
      --current_exception_count;}
    else focus++;
  if (unwind_stack_before && !exceptions.size()) unwind_stack_v = false;
  return ret;}

void Call_stack::replace_error(void) {++current_exception_count;}

void Call_stack::request_exit(int exit_val) {
  exit_requested = true;
  if (!in_exception_handler()) unwind_stack_v = true;
  exit_v += exit_val;}

void Call_stack::reset(void) {
  unwind_stack_v = false;
  current_exception_count = 0;}

Conditional_state::Conditional_state(void) : in_if_block(false),
    successful_condition(false), exception_thrown(false) {}
