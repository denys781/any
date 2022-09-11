#pragma once

#include <memory>
#include <utility>
#include <typeinfo>
#include <type_traits>

namespace experimental
{
	template <class _Ty> // from C++17 utility header file
	struct in_place_type_t final
	{
		explicit in_place_type_t() noexcept = default;
	};

	class any
	{
	public:
		template <class _Ty>
		friend _Ty* any_cast(any*) noexcept;

		template <class _Ty>
		friend const _Ty* any_cast(const any*) noexcept;

		any() noexcept = default;
		any(any&&) noexcept = default;
		any& operator=(any&&) noexcept = default;

		any(const any& other)
			: m_instance(other.m_instance->clone())
		{ }

		any& operator=(const any& other)
		{
			if (this != &other) {
				m_instance = other.m_instance->clone();
			}
			return *this;
		}

		template <class _Ty, class = std::enable_if_t<
			!std::is_same<std::decay_t<_Ty>, any>::value &&
			std::is_copy_constructible<std::decay_t<_Ty>>::value>>
		any(_Ty&& value)
			: m_instance(std::make_unique<storage<std::decay_t<_Ty>>>(std::forward<_Ty>(value)))
		{ }

		template <class _Ty, class = std::enable_if_t<
			!std::is_same<std::decay_t<_Ty>, any>::value &&
			std::is_copy_constructible<std::decay_t<_Ty>>::value>>
		any& operator=(_Ty&& value)
		{
			m_instance = std::make_unique<storage<std::decay_t<_Ty>>>(std::forward<_Ty>(value));
			return *this;
		}

		template <class _Ty, class... _Args, class = std::enable_if_t<
			std::is_copy_constructible<std::decay_t<_Ty>>::value &&
			std::is_constructible<std::decay_t<_Ty>, _Args...>::value>>
		explicit any(in_place_type_t<_Ty>, _Args&&... args)
			: m_instance(std::make_unique<storage<std::decay_t<_Ty>>>(std::forward<_Args>(args)...))
		{ }

		void reset() noexcept
		{
			m_instance.reset();
		}

		bool has_value() const noexcept
		{
			return static_cast<bool>(m_instance);
		}

		const std::type_info& type() const noexcept
		{
			return m_instance ? m_instance->type() : typeid(void);
		}
	private:
		struct storage_base
		{
			virtual ~storage_base() = default;
			virtual const std::type_info& type() const noexcept = 0;
			virtual std::unique_ptr<storage_base> clone() const = 0;
		};

		template <class _Ty>
		struct storage final : storage_base
		{
			template <class... _Args>
			storage(_Args&&... args)
				: m_value(std::forward<_Args>(args)...)
			{ }

			const std::type_info& type() const noexcept override
			{
				return typeid(_Ty);
			}

			std::unique_ptr<storage_base> clone() const override
			{                                             
				using return_type_t = std::unique_ptr<storage_base>;
				return return_type_t(std::make_unique<storage>(*this));
			}

			_Ty m_value;
		};

		std::unique_ptr<storage_base> m_instance;
	};

	template <class _Ty, class... _Args, class = std::enable_if_t<
		std::is_constructible<any, in_place_type_t<_Ty>, _Args...>::value>>
	any make_any(_Args&&... args)
	{
		return any(in_place_type_t<_Ty>(), std::forward<_Args>(args)...);
	}

	template <class _Ty>
	_Ty* any_cast(any* anything) noexcept
	{
		if (anything) {
			if (anything->has_value() && anything->type() == typeid(_Ty)) {
				auto storage = static_cast<any::storage<_Ty>*>(anything->m_instance.get());
				return &storage->m_value;
			}
		}
		return nullptr;
	}

	template <class _Ty>
	const _Ty* any_cast(const any* anything) noexcept
	{
		if (anything) {
			if (anything->has_value() && anything->type() == typeid(_Ty)) {
				const auto storage = static_cast<const any::storage<_Ty>*>(anything->m_instance.get());
				return &storage->m_value;
			}
		}
		return nullptr;
	}
}