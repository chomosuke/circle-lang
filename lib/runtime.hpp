// #pragma once
//
// #include "macros.hpp"
// #include "number.hpp"
// #include "parser.hpp"
//
// #include <memory>
//
// namespace runtime {
//     class Array;
//     class Assign;
//     class Index;
//     class OperatorBinary;
//     class OperatorUnary;
//     class Number;
//
//     class Obj {
//       public:
//         Obj(const Obj&) = delete;
//         Obj(Obj&&) = delete;
//         Obj& operator=(const Obj&) = delete;
//         Obj& operator=(Obj&&) = delete;
//         virtual ~Obj() = default;
//         virtual void execute() = 0;
//         virtual std::unique_ptr<Obj> evaluate() = 0;
//     };
//
//     class Assign : Obj {
//       public:
//         void execute() override;
//         std::unique_ptr<Obj> evaluate() override;
//     };
//
//     class Index : Obj {
//       public:
//         void execute() override;
//         std::unique_ptr<Obj> evaluate() override;
//     };
//
//     class OperatorBinary : Obj {
//       public:
//         void execute() override;
//         std::unique_ptr<Obj> evaluate() override;
//     };
//
//     class OperatorUnary : Obj {
//       public:
//         void execute() override;
//         std::unique_ptr<Obj> evaluate() override;
//     };
//
//     class Number : Obj {
//       private:
//         number::Value m_value;
//
//       public:
//         void execute() override;
//         std::unique_ptr<Obj> evaluate() override;
//     };
//
//     class Array : Obj {
//       private:
//         int m_length;
//
//       public:
//         explicit Array(int length);
//         void insert(number::Value v);
//
//         void execute() override;
//         std::unique_ptr<Obj> evaluate() override;
//     };
//
//     class Runtime {
//       private:
//         Array m_global_array;
//         Array m_code;
//
//       public:
//         explicit Runtime(ast::Array&& code);
//         void run(std::istream& in, std::ostream& out, std::ostream& err);
//     };
// } // namespace runtime
