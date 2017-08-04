#if !defined(Column_h)
#define Column_h

/* Column is a type used to declare the purpose of what is stored */
template <const char* LABEL, typename T>
  struct Column{
    using type = T;
    static constexpr char const * const kLabel = LABEL;

    static const char* const& label() {
      static char const* const s_label(LABEL);
      return s_label;
    }
  };

#endif
