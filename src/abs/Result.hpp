//
// Created by mike on 25-7-17.
//
#pragma once

#include <string>
#include <utility>

    template<typename T, typename E = std::string>
    class Result {
    public:
        using ValueType = T;
        using ErrorType = E;

        // Factory: Ok
        static Result Ok(T value) {
            return Result(std::move(value));
        }

        // Factory: Error
        static Result Err(E error) {
            return Result(std::move(error));
        }

        // State checks
        [[nodiscard]] bool isOk() const noexcept {
            return std::holds_alternative<T>(data_);
        }

        [[nodiscard]] bool isErr() const noexcept {
            return std::holds_alternative<E>(data_);
        }

        // Access value
        const T& value() const {
            if (isErr()) throw std::logic_error("Tried to access value of an error Result");
            return std::get<T>(data_);
        }

        T& value() {
            if (isErr()) throw std::logic_error("Tried to access value of an error Result");
            return std::get<T>(data_);
        }

        // Access error
        const E& error() const {
            if (isOk()) throw std::logic_error("Tried to access error of an ok Result");
            return std::get<E>(data_);
        }

        E& error() {
            if (isOk()) throw std::logic_error("Tried to access error of an ok Result");
            return std::get<E>(data_);
        }

    private:
        std::variant<T, E> data_;

        explicit Result(T value) : data_(std::move(value)) {}
        explicit Result(E error) : data_(std::move(error)) {}
    };


    // -------------------------
    // Specialization: Result<void, E>
    // -------------------------
    template<typename E>
    class Result<void, E> {
    public:
        using ValueType = void;
        using ErrorType = E;

        static Result Ok() {
            return Result(true, E{});
        }

        static Result Err(E error) {
            return Result(false, std::move(error));
        }

        [[nodiscard]] bool isOk() const noexcept {
            return ok_;
        }

        [[nodiscard]] bool isErr() const noexcept {
            return !ok_;
        }

        const E& error() const {
            if (ok_) throw std::logic_error("Tried to access error of an ok Result<void>");
            return error_;
        }

        E& error() {
            if (ok_) throw std::logic_error("Tried to access error of an ok Result<void>");
            return error_;
        }

    private:
        bool ok_;
        E error_;
        Result( bool ok, E error) : ok_(ok), error_(std::move(error)) {}
    };