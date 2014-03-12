#pragma once
#ifndef PERSISTENCE_CONNECTION_HPP_INCLUDED
#define PERSISTENCE_CONNECTION_HPP_INCLUDED

#include <persistence/result_set.hpp>
#include <persistence/query.hpp>

namespace persistence {
  struct IConnection {
    virtual ~IConnection() {}

    // Info
    virtual std::string database() const = 0;
    virtual std::string user() const = 0;
    virtual std::string host() const = 0;

    // Queries
    virtual std::string to_sql(const ast::IQuery& q) = 0;
    virtual std::string sanitize(std::string input) = 0;
    virtual std::unique_ptr<IResultSet> execute(std::string sql) = 0;
    virtual std::unique_ptr<IResultSet> execute(const ast::IQuery& query) = 0;
  };
}

#endif // PERSISTENCE_CONNECTION_HPP_INCLUDED
