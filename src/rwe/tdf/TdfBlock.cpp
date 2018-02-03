#include "TdfBlock.h"

#include <rwe/rwe_string.h>

namespace rwe
{
    template<>
    boost::optional<int> tdfTryParse<int>(const std::string& value)
    {
        try
        {
            return std::stoi(value);
        }
        catch (const std::invalid_argument& e)
        {
            return boost::none;
        }
    }

    template<>
    boost::optional<unsigned int> tdfTryParse<unsigned int>(const std::string& value)
    {
        try
        {
            return std::stoul(value);
        }
        catch (const std::invalid_argument& e)
        {
            return boost::none;
        }
    }

    template<>
    boost::optional<float> tdfTryParse<float>(const std::string& value)
    {
        try
        {
            return std::stof(value);
        }
        catch (const std::invalid_argument& e)
        {
            return boost::none;
        }
    }

    template<>
    boost::optional<bool> tdfTryParse<bool>(const std::string& value)
    {
        int i;
        try
        {
            i = std::stoi(value);
        }
        catch (const std::invalid_argument& e)
        {
            return boost::none;
        }

        return i != 0;
    }

    template<>
    boost::optional<std::string> tdfTryParse<std::string>(const std::string& value)
    {
        return value;
    }

    class EqualityVisitor : public boost::static_visitor<bool>
    {
    private:
        const TdfPropertyValue* other;

    public:
        explicit EqualityVisitor(const TdfPropertyValue* other) : other(other) {}
        bool operator()(const std::string& s) const
        {
            auto rhs = boost::get<std::string>(other);
            if (!rhs)
            {
                return false;
            }

            return s == *rhs;
        }

        bool operator()(const TdfBlock& b) const
        {
            auto rhs = boost::get<TdfBlock>(other);
            if (!rhs)
            {
                return false;
            }

            return b == *rhs;
        }
    };

    TdfValueException::TdfValueException(const std::string& message) : runtime_error(message)
    {
    }

    TdfValueException::TdfValueException(const char* message) : runtime_error(message)
    {
    }

    TdfBlockEntry::TdfBlockEntry(std::string name, const std::string& value) : name(std::move(name)), value(std::make_unique<TdfPropertyValue>(value)) {}

    TdfBlockEntry::TdfBlockEntry(std::string name, TdfBlock block) : name(std::move(name)), value(std::make_unique<TdfPropertyValue>(std::move(block))) {}

    TdfBlockEntry::TdfBlockEntry(std::string name) : name(std::move(name)), value(std::make_unique<TdfPropertyValue>(TdfBlock())) {}

    TdfBlockEntry::TdfBlockEntry(const TdfBlockEntry& other) : name(other.name), value(std::make_unique<TdfPropertyValue>(*other.value))
    {
    }

    bool TdfBlockEntry::operator==(const TdfBlockEntry& rhs) const
    {
        if (name != rhs.name)
        {
            return false;
        }

        return boost::apply_visitor(EqualityVisitor(rhs.value.get()), *value);
    }

    bool TdfBlockEntry::operator!=(const TdfBlockEntry& rhs) const
    {
        return !(rhs == *this);
    }

    TdfBlockEntry::TdfBlockEntry(std::string name, std::vector<TdfBlockEntry> entries) : TdfBlockEntry(std::move(name), TdfBlock(std::move(entries)))
    {
    }

    std::ostream& operator<<(std::ostream& os, const TdfBlockEntry& entry)
    {
        auto leaf = boost::get<std::string>(&*entry.value);
        if (leaf != nullptr)
        {
            os << entry.name << " = " << *leaf << ";";
        }
        else
        {
            auto block = boost::get<TdfBlock>(&*entry.value);
            if (block != nullptr)
            {
                os << "[" << entry.name << "]{ ";
                for (auto elem : block->entries)
                {
                    os << elem << " ";
                }
                os << "}";
            }
            else
            {
                throw std::logic_error("Value was neither primitive nor block");
            }
        }

        return os;
    }

    bool TdfBlock::operator==(const TdfBlock& rhs) const
    {
        return entries == rhs.entries;
    }

    bool TdfBlock::operator!=(const TdfBlock& rhs) const
    {
        return !(rhs == *this);
    }

    boost::optional<const TdfBlock&> TdfBlock::findBlock(const std::string& name) const
    {
        auto pos = entries.begin();
        for (;;)
        {
            // find the key in the block
            pos = std::find_if(pos, entries.end(), [name](const TdfBlockEntry& e) { return toUpper(e.name) == toUpper(name); });
            if (pos == entries.end())
            {
                return boost::none;
            }

            // make sure the key contains a block and extract it
            auto& valuePointer = pos->value;
            auto ptr = boost::get<TdfBlock>(&*valuePointer);
            if (ptr == nullptr)
            {
                // the key didn't contain a block, keep looking
                ++pos;
                continue;
            }

            return *ptr;
        }
    }

    boost::optional<const std::string&> TdfBlock::findValue(const std::string& name) const
    {
        // find the key in the block
        auto pos = std::find_if(entries.begin(), entries.end(), [name](const TdfBlockEntry& e) { return toUpper(e.name) == toUpper(name); });
        if (pos == entries.end())
        {
            return boost::none;
        }

        // make sure the key contains a primitive (not a block) and extract it
        auto& valuePointer = pos->value;
        auto ptr = boost::get<std::string>(&*valuePointer);
        if (ptr == nullptr)
        {
            return boost::none;
        }

        return *ptr;
    }

    TdfBlock::TdfBlock(std::vector<TdfBlockEntry>&& entries) : entries(std::move(entries))
    {
    }

    boost::optional<int> TdfBlock::extractInt(const std::string& key) const
    {
        return extract<int>(key);
    }

    boost::optional<unsigned int> TdfBlock::extractUint(const std::string& key) const
    {
        return extract<unsigned int>(key);
    }

    boost::optional<float> TdfBlock::extractFloat(const std::string& key) const
    {
        return extract<float>(key);
    }

    std::string TdfBlock::expectString(const std::string& key) const
    {
        return expect<std::string>(key);
    }

    int TdfBlock::expectInt(const std::string& key) const
    {
        return expect<int>(key);
    }

    float TdfBlock::expectFloat(const std::string& key) const
    {
        return expect<float>(key);
    }

    boost::optional<bool> TdfBlock::extractBool(const std::string& key) const
    {
        return extract<bool>(key);
    }

    bool TdfBlock::expectBool(const std::string& key) const
    {
        return expect<bool>(key);
    }

    unsigned int TdfBlock::expectUint(const std::string& key) const
    {
        return expect<unsigned int>(key);
    }
}
