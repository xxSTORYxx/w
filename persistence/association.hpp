#pragma once
#ifndef PERSISTENCE_ASSOCIATIONS_HPP_INCLUDED
#define PERSISTENCE_ASSOCIATIONS_HPP_INCLUDED

#include <persistence/property.hpp>
#include <persistence/record_ptr.hpp>

#include <wayward/support/error.hpp>

namespace persistence {
  struct IRecordType;
  struct Context;

  struct AssociationError : wayward::Error {
    AssociationError(const std::string& msg) : wayward::Error(msg) {}
  };

  // Descriptors for the type reflection system:
  struct IAssociation {
    virtual ~IAssociation() {}
    virtual const IRecordType* self_type() const = 0;
    virtual const IRecordType* foreign_type() const = 0;
    virtual std::string foreign_key() const = 0;
  };

  // Base class for things like BelongsTo<>, HasMany<>, etc.
  struct IAssociationAnchor {
    virtual ~IAssociationAnchor() {}
    virtual void initialize(const IAssociation* association, Context* context) = 0;
    virtual void load() = 0;
    virtual bool is_loaded() const = 0;
    virtual const IAssociation* association() const = 0;
    virtual Context* context() const = 0;
  };

  template <class T>
  struct ISingularAssociationAnchor : IAssociationAnchor {
    using AssociatedType = T;
    virtual void populate(RecordPtr<T> record) = 0;
    virtual RecordPtr<T> get() const = 0;
  };

  template <class T>
  struct IPluralAssociationAnchor : IAssociationAnchor {
    using AssociatedType = T;
    virtual void populate(std::vector<RecordPtr<T>> records) = 0;
    virtual std::vector<RecordPtr<T>> get() = 0;
  };

  template <class T, class Base>
  struct AssociationAnchorBase : Base {
    const IAssociation* association_;
    Context* context_;

    const IAssociation* association() const final { return association_; }
    Context* context() const final { return context_; }

    void initialize(const IAssociation* association, Context* context) override {
      association_ = association;
      context_ = context;
    }
  };

  template <class T>
  struct SingularAssociationAnchor : AssociationAnchorBase<T, ISingularAssociationAnchor<T>> {
  };

  template <class T>
  struct PluralAssociationAnchor : AssociationAnchorBase<T, IPluralAssociationAnchor<T>> {
  };

  template <class Owner>
  struct IAssociationFrom : IAssociation {
    virtual void initialize_in_object(Owner& object, Context* context) const = 0;
    virtual IAssociationAnchor* get_anchor(Owner& object) const = 0;
    virtual const IAssociationAnchor* get_anchor(const Owner& object) const = 0;
    virtual wayward::data_franca::ReaderPtr get_member_reader(const Owner& object, wayward::Bitflags<wayward::data_franca::Options>) const = 0;
    virtual wayward::data_franca::AdapterPtr get_member_adapter(Owner& object, wayward::Bitflags<wayward::data_franca::Options>) const = 0;
  };

  template <class Owner>
  struct AssociationFrom : IAssociationFrom<Owner> {
    const IRecordType* self_type() const { return get_type<Owner>(); }
  };

  template <class Owner_, class T_>
  struct AssociationBetween : IAssociationFrom<Owner_> {
    using Owner = Owner_;
    using AssociatedType = T_;
    const IRecordType* foreign_type() const { return get_type<T_>(); }
  };

  template <class Anchor, class Base>
  struct AssociationBase : Base {
    using Owner = typename Base::Owner;
    using AssociatedType = typename Base::AssociatedType;
    using MemberPtr = Anchor Owner::*;
    AssociationBase(std::string key, MemberPtr member) : key_(std::move(key)), member_(member) {}

    Anchor* get(Owner& object) const { return &(object.*member_); }
    const Anchor* get(const Owner& object) const { return &(object.*member_); }

    IAssociationAnchor* get_anchor(Owner& object) const final { return get(object); }
    const IAssociationAnchor* get_anchor(const Owner& object) const final { return get(object); }

    void initialize_in_object(Owner& object, Context* context) const {
      get(object)->initialize(this, context);
    }

    wayward::data_franca::ReaderPtr get_member_reader(const Owner& object, wayward::Bitflags<wayward::data_franca::Options> options) const final {
      return wayward::data_franca::make_reader(*get(object), options);
    }

    wayward::data_franca::AdapterPtr get_member_adapter(Owner& object, wayward::Bitflags<wayward::data_franca::Options> options) const final {
      return wayward::data_franca::make_adapter(*get(object), options);
    }

    MemberPtr member_ptr() const { return member_; }

    const IRecordType* foreign_type() const final { return get_type<AssociatedType>(); }
    const IRecordType* self_type() const final { return get_type<Owner>(); }
    std::string foreign_key() const final { return key_; }

  protected:
    std::string key_;
    MemberPtr member_;
  };

  template <class Owner, class Anchor>
  struct SingularAssociationBase : AssociationBase<Anchor, AssociationBetween<Owner, typename Anchor::AssociatedType>> {
    using BaseClass = AssociationBase<Anchor, AssociationBetween<Owner, typename Anchor::AssociatedType>>;
    using MemberPtr = typename BaseClass::MemberPtr;
    SingularAssociationBase(std::string key, MemberPtr member) : BaseClass(std::move(key), member) {}
  };

  template <class Owner, class Anchor>
  struct PluralAssociationBase : AssociationBase<Anchor, AssociationBetween<Owner, typename Anchor::AssociatedType>> {
    using BaseClass = AssociationBase<Anchor, AssociationBetween<Owner, typename Anchor::AssociatedType>>;
    using MemberPtr = typename BaseClass::MemberPtr;
    PluralAssociationBase(std::string key, MemberPtr member) : BaseClass(std::move(key), member) {}
  };
}

#endif // PERSISTENCE_ASSOCIATIONS_HPP_INCLUDED
