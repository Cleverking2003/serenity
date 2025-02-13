/*
 * Copyright (c) 2021, Ali Mohammad Pur <mpfard@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <LibWasm/AbstractMachine/Configuration.h>
#include <LibWasm/AbstractMachine/Interpreter.h>

namespace Wasm {

struct BytecodeInterpreter : public Interpreter {
    virtual void interpret(Configuration&) override;
    virtual ~BytecodeInterpreter() override = default;
    virtual bool did_trap() const override { return m_trap.has_value(); }
    virtual String trap_reason() const override { return m_trap.value().reason; }
    virtual void clear_trap() override { m_trap.clear(); }

    struct CallFrameHandle {
        explicit CallFrameHandle(BytecodeInterpreter& interpreter, Configuration& configuration)
            : m_configuration_handle(configuration)
            , m_interpreter(interpreter)
        {
        }

        ~CallFrameHandle() = default;

        Configuration::CallFrameHandle m_configuration_handle;
        BytecodeInterpreter& m_interpreter;
    };

protected:
    virtual void interpret(Configuration&, InstructionPointer&, Instruction const&);
    void branch_to_label(Configuration&, LabelIndex);
    template<typename ReadT, typename PushT>
    void load_and_push(Configuration&, Instruction const&);
    void store_to_memory(Configuration&, Instruction const&, ReadonlyBytes data);
    void call_address(Configuration&, FunctionAddress);

    template<typename V, typename T>
    MakeUnsigned<T> checked_unsigned_truncate(V);

    template<typename V, typename T>
    MakeSigned<T> checked_signed_truncate(V);

    template<typename T>
    T read_value(ReadonlyBytes data);

    Vector<Value> pop_values(Configuration& configuration, size_t count);
    bool trap_if_not(bool value, String reason)
    {
        if (!value)
            m_trap = Trap { move(reason) };
        return m_trap.has_value();
    }

    Optional<Trap> m_trap;
};

struct DebuggerBytecodeInterpreter : public BytecodeInterpreter {
    virtual ~DebuggerBytecodeInterpreter() override = default;

    Function<bool(Configuration&, InstructionPointer&, Instruction const&)> pre_interpret_hook;
    Function<bool(Configuration&, InstructionPointer&, Instruction const&, Interpreter const&)> post_interpret_hook;

private:
    virtual void interpret(Configuration&, InstructionPointer&, Instruction const&) override;
};

}
