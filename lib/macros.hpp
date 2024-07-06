#define NON_COPIABLE(typename_)                                                                    \
    typename_(const typename_&) = delete;                                                          \
    typename_& operator=(const typename_&) = delete;                                               \
    typename_& operator=(typename_&&) noexcept = default;                                          \
    typename_(typename_&& o) noexcept = default;                                                   \
    ~typename_() = default;
