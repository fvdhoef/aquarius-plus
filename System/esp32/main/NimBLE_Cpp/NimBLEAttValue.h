/*
 * NimBLEAttValue.h
 *
 *  Created: on March 18, 2021
 *      Author H2zero
 *
 */

#pragma once

#include "nimconfig.h"
#include "NimBLELog.h"
#include <string>
#include <vector>

#if !defined(CONFIG_NIMBLE_CPP_ATT_VALUE_INIT_LENGTH)
#    define CONFIG_NIMBLE_CPP_ATT_VALUE_INIT_LENGTH 20
#elif CONFIG_NIMBLE_CPP_ATT_VALUE_INIT_LENGTH > BLE_ATT_ATTR_MAX_LEN
#    error CONFIG_NIMBLE_CPP_ATT_VALUE_INIT_LENGTH cannot be larger than 512 (BLE_ATT_ATTR_MAX_LEN)
#elif CONFIG_NIMBLE_CPP_ATT_VALUE_INIT_LENGTH < 1
#    error CONFIG_NIMBLE_CPP_ATT_VALUE_INIT_LENGTH cannot be less than 1; Range = 1 : 512
#endif

/* Used to determine if the type passed to a template has a c_str() and length() method. */
template <typename T, typename = void, typename = void>
struct Has_c_str_len : std::false_type {};

template <typename T>
struct Has_c_str_len<T, decltype(void(std::declval<T &>().c_str())), decltype(void(std::declval<T &>().length()))> : std::true_type {};

/**
 * @brief A specialized container class to hold BLE attribute values.
 * @details This class is designed to be more memory efficient than using\n
 * standard container types for value storage, while being convertible to\n
 * many different container classes.
 */
class NimBLEAttValue {
    uint8_t *m_attr_value   = nullptr;
    uint16_t m_attr_max_len = 0;
    uint16_t m_attr_len     = 0;
    uint16_t m_capacity     = 0;
    void     deepCopy(const NimBLEAttValue &source);

public:
    /**
     * @brief Default constructor.
     * @param[in] init_len The initial size in bytes.
     * @param[in] max_len The max size in bytes that the value can be.
     */
    NimBLEAttValue(uint16_t init_len = CONFIG_NIMBLE_CPP_ATT_VALUE_INIT_LENGTH, uint16_t max_len = BLE_ATT_ATTR_MAX_LEN);

    /**
     * @brief Construct with an initial value from a buffer.
     * @param value A pointer to the initial value to set.
     * @param[in] len The size in bytes of the value to set.
     * @param[in] max_len The max size in bytes that the value can be.
     */
    NimBLEAttValue(const uint8_t *value, uint16_t len, uint16_t max_len = BLE_ATT_ATTR_MAX_LEN);

    /**
     * @brief Construct with an initializer list.
     * @param list An initializer list containing the initial value to set.
     * @param[in] max_len The max size in bytes that the value can be.
     */
    NimBLEAttValue(std::initializer_list<uint8_t> list, uint16_t max_len = BLE_ATT_ATTR_MAX_LEN)
        : NimBLEAttValue(list.begin(), (uint16_t)list.size(), max_len) {
    }

    /**
     * @brief Construct with an initial value from a const char string.
     * @param value A pointer to the initial value to set.
     * @param[in] max_len The max size in bytes that the value can be.
     */
    NimBLEAttValue(const char *value, uint16_t max_len = BLE_ATT_ATTR_MAX_LEN)
        : NimBLEAttValue((uint8_t *)value, (uint16_t)strlen(value), max_len) {
    }

    /**
     * @brief Construct with an initial value from a std::string.
     * @param str A std::string containing to the initial value to set.
     * @param[in] max_len The max size in bytes that the value can be.
     */
    NimBLEAttValue(const std::string str, uint16_t max_len = BLE_ATT_ATTR_MAX_LEN)
        : NimBLEAttValue((uint8_t *)str.data(), (uint16_t)str.length(), max_len) {
    }

    /**
     * @brief Construct with an initial value from a std::vector<uint8_t>.
     * @param vec A std::vector<uint8_t> containing to the initial value to set.
     * @param[in] max_len The max size in bytes that the value can be.
     */
    NimBLEAttValue(const std::vector<uint8_t> vec, uint16_t max_len = BLE_ATT_ATTR_MAX_LEN)
        : NimBLEAttValue(&vec[0], (uint16_t)vec.size(), max_len) {
    }

    /** @brief Copy constructor */
    NimBLEAttValue(const NimBLEAttValue &source) {
        deepCopy(source);
    }

    /** @brief Move constructor */
    NimBLEAttValue(NimBLEAttValue &&source) {
        *this = std::move(source);
    }

    /** @brief Destructor */
    ~NimBLEAttValue();

    /** @brief Returns the max size in bytes */
    uint16_t max_size() const {
        return m_attr_max_len;
    }

    /** @brief Returns the currently allocated capacity in bytes */
    uint16_t capacity() const {
        return m_capacity;
    }

    /** @brief Returns the current length of the value in bytes */
    uint16_t length() const {
        return m_attr_len;
    }

    /** @brief Returns the current size of the value in bytes */
    uint16_t size() const {
        return m_attr_len;
    }

    /** @brief Returns a pointer to the internal buffer of the value */
    const uint8_t *data() const {
        return m_attr_value;
    }

    /** @brief Returns a pointer to the internal buffer of the value as a const char* */
    const char *c_str() const {
        return (const char *)m_attr_value;
    }

    /** @brief Iterator begin */
    const uint8_t *begin() const {
        return m_attr_value;
    }

    /** @brief Iterator end */
    const uint8_t *end() const {
        return m_attr_value + m_attr_len;
    }

    /**
     * @brief Set the value from a buffer
     * @param[in] value A ponter to a buffer containing the value.
     * @param[in] len The length of the value in bytes.
     * @returns True if successful.
     */
    bool setValue(const uint8_t *value, uint16_t len);

    /**
     * @brief Set value to the value of const char*.
     * @param [in] s A ponter to a const char value to set.
     */
    bool setValue(const char *s) {
        return setValue((uint8_t *)s, (uint16_t)strlen(s));
    }

    /**
     * @brief Get a pointer to the value buffer
     * @returns A pointer to the internal value buffer.
     */
    const uint8_t *getValue();

    /**
     * @brief Append data to the value.
     * @param[in] value A ponter to a data buffer with the value to append.
     * @param[in] len The length of the value to append in bytes.
     * @returns A reference to the appended NimBLEAttValue.
     */
    NimBLEAttValue &append(const uint8_t *value, uint16_t len);

    /*********************** Template Functions ************************/

    /**
     * @brief Template to set value to the value of <type\>val.
     * @param [in] s The <type\>value to set.
     * @details Only used for types without a `c_str()` method.
     */
    template <typename T>
    typename std::enable_if<!Has_c_str_len<T>::value, bool>::type
    setValue(const T &s) {
        return setValue((uint8_t *)&s, sizeof(T));
    }

    /**
     * @brief Template to set value to the value of <type\>val.
     * @param [in] s The <type\>value to set.
     * @details Only used if the <type\> has a `c_str()` method.
     */
    template <typename T>
    typename std::enable_if<Has_c_str_len<T>::value, bool>::type
    setValue(const T &s) {
        return setValue((uint8_t *)s.c_str(), (uint16_t)s.length());
    }

    /**
     * @brief Template to return the value as a <type\>.
     * @tparam T The type to convert the data to.
     * @param [in] skipSizeCheck If true it will skip checking if the data size is less than\n
     * <tt>sizeof(<type\>)</tt>.
     * @return The data converted to <type\> or NULL if skipSizeCheck is false and the data is\n
     * less than <tt>sizeof(<type\>)</tt>.
     * @details <b>Use:</b> <tt>getValue<type>(skipSizeCheck);</tt>
     */
    template <typename T>
    T getValue(bool skipSizeCheck = false) {
        if (!skipSizeCheck && size() < sizeof(T)) {
            return T();
        }
        return *((T *)getValue());
    }

    /*********************** Operators ************************/

    /** @brief Subscript operator */
    uint8_t operator[](int pos) const {
        assert(pos < m_attr_len && "out of range");
        return m_attr_value[pos];
    }

    /** @brief Operator; Get the value as a std::vector<uint8_t>. */
    operator std::vector<uint8_t>() const {
        return std::vector<uint8_t>(m_attr_value, m_attr_value + m_attr_len);
    }

    /** @brief Operator; Get the value as a std::string. */
    operator std::string() const {
        return std::string((char *)m_attr_value, m_attr_len);
    }

    /** @brief Operator; Get the value as a const uint8_t*. */
    operator const uint8_t *() const {
        return m_attr_value;
    }

    /** @brief Operator; Append another NimBLEAttValue. */
    NimBLEAttValue &operator+=(const NimBLEAttValue &source) {
        return append(source.data(), source.size());
    }

    /** @brief Operator; Set the value from a std::string source. */
    NimBLEAttValue &operator=(const std::string &source) {
        setValue((uint8_t *)source.data(), (uint16_t)source.size());
        return *this;
    }

    /** @brief Move assignment operator */
    NimBLEAttValue &operator=(NimBLEAttValue &&source);

    /** @brief Copy assignment operator */
    NimBLEAttValue &operator=(const NimBLEAttValue &source);

    /** @brief Equality operator */
    bool operator==(const NimBLEAttValue &source) {
        return (m_attr_len == source.size()) ? memcmp(m_attr_value, source.data(), m_attr_len) == 0 : false;
    }

    /** @brief Inequality operator */
    bool operator!=(const NimBLEAttValue &source) {
        return !(*this == source);
    }
};

inline NimBLEAttValue::NimBLEAttValue(uint16_t init_len, uint16_t max_len) {
    m_attr_value = (uint8_t *)calloc(init_len + 1, 1);
    assert(m_attr_value && "No Mem");
    m_attr_max_len = std::min(BLE_ATT_ATTR_MAX_LEN, (int)max_len);
    m_attr_len     = 0;
    m_capacity     = init_len;
}

inline NimBLEAttValue::NimBLEAttValue(const uint8_t *value, uint16_t len, uint16_t max_len)
    : NimBLEAttValue(len, max_len) {
    memcpy(m_attr_value, value, len);
    m_attr_value[len] = '\0';
    m_attr_len        = len;
}

inline NimBLEAttValue::~NimBLEAttValue() {
    if (m_attr_value != nullptr) {
        free(m_attr_value);
    }
}

inline NimBLEAttValue &NimBLEAttValue::operator=(NimBLEAttValue &&source) {
    if (this != &source) {
        free(m_attr_value);

        m_attr_value        = source.m_attr_value;
        m_attr_max_len      = source.m_attr_max_len;
        m_attr_len          = source.m_attr_len;
        m_capacity          = source.m_capacity;
        source.m_attr_value = nullptr;
    }
    return *this;
}

inline NimBLEAttValue &NimBLEAttValue::operator=(const NimBLEAttValue &source) {
    if (this != &source) {
        deepCopy(source);
    }
    return *this;
}

inline void NimBLEAttValue::deepCopy(const NimBLEAttValue &source) {
    uint8_t *res = (uint8_t *)realloc(m_attr_value, source.m_capacity + 1);
    assert(res && "deepCopy: realloc failed");

    ble_npl_hw_enter_critical();
    m_attr_value   = res;
    m_attr_max_len = source.m_attr_max_len;
    m_attr_len     = source.m_attr_len;
    m_capacity     = source.m_capacity;
    memcpy(m_attr_value, source.m_attr_value, m_attr_len + 1);
    ble_npl_hw_exit_critical(0);
}

inline const uint8_t *NimBLEAttValue::getValue() {
    return m_attr_value;
}

inline bool NimBLEAttValue::setValue(const uint8_t *value, uint16_t len) {
    if (len > m_attr_max_len) {
        NIMBLE_LOGE("NimBLEAttValue", "value exceeds max, len=%u, max=%u", len, m_attr_max_len);
        return false;
    }

    uint8_t *res = m_attr_value;
    if (len > m_capacity) {
        res        = (uint8_t *)realloc(m_attr_value, (len + 1));
        m_capacity = len;
    }
    assert(res && "setValue: realloc failed");

    ble_npl_hw_enter_critical();
    m_attr_value = res;
    memcpy(m_attr_value, value, len);
    m_attr_value[len] = '\0';
    m_attr_len        = len;
    ble_npl_hw_exit_critical(0);
    return true;
}

inline NimBLEAttValue &NimBLEAttValue::append(const uint8_t *value, uint16_t len) {
    if (len < 1) {
        return *this;
    }

    if ((m_attr_len + len) > m_attr_max_len) {
        NIMBLE_LOGE("NimBLEAttValue", "val > max, len=%u, max=%u", len, m_attr_max_len);
        return *this;
    }

    uint8_t *res     = m_attr_value;
    uint16_t new_len = m_attr_len + len;
    if (new_len > m_capacity) {
        res        = (uint8_t *)realloc(m_attr_value, (new_len + 1));
        m_capacity = new_len;
    }
    assert(res && "append: realloc failed");

    ble_npl_hw_enter_critical();
    m_attr_value = res;
    memcpy(m_attr_value + m_attr_len, value, len);
    m_attr_len               = new_len;
    m_attr_value[m_attr_len] = '\0';
    ble_npl_hw_exit_critical(0);

    return *this;
}
