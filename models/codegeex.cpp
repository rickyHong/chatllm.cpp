namespace v2
{
struct Config : public glm::v2::Config
{
};

class ChatHistoryEncoder : public BaseHistoryEncoder
{
public:
    void do_append_user(int round_idx, const std::string &user, std::vector<int> &ids) const override;
};

static ChatHistoryEncoder _chat_encoder;

class Tokenizer : public glm::v2::Tokenizer
{
public:
    Tokenizer(const Config &config) : glm::v2::Tokenizer::Tokenizer(config, &_chat_encoder)
    {
        sys_prompt = "# language: Python";
    }
};

class ConditionalGeneration : public glm::v2::ConditionalGeneration
{
public:
    ConditionalGeneration() = default;
    ConditionalGeneration(const Config &config)
        : glm::v2::ConditionalGeneration(config, MODEL_TYPE_CODEGEEX2)
    {
    }
};

void ChatHistoryEncoder::do_append_user(int round_idx, const std::string &user, std::vector<int> &ids) const
{
    std::string combined = tokenizer->get_system_prompt() + "\n" + user + "\n";
    tokenizer->encode(combined, ids);
}
}

namespace v4
{
typedef glm::v4::Config Config;

class Tokenizer : public glm::v4::Tokenizer
{
public:
    Tokenizer(const Config &config) : glm::v4::Tokenizer(config)
    {}

    size_t load(tokenizer::DataReader *buffer, int n_vocab) override
    {
        size_t r = glm::v4::Tokenizer::load(buffer, n_vocab);
        int special_id = observation_token_id + 5;
        code_prefix_token_id = special_id++;
        code_middle_token_id = special_id++;
        code_suffix_token_id = special_id++;
        cursor_token_id      = special_id++;
        tp->AddAddedToken("<|code_prefix|>", code_prefix_token_id);
        tp->AddAddedToken("<|code_middle|>", code_middle_token_id);
        tp->AddAddedToken("<|code_suffix|>", code_suffix_token_id);
        tp->AddAddedToken("<|cursor|>",      cursor_token_id);
        return r;
    }
public:
    int code_prefix_token_id;
    int code_middle_token_id;
    int code_suffix_token_id;
    int cursor_token_id;
};

class ConditionalGeneration : public glm::v4::ConditionalGeneration
{
public:
    ConditionalGeneration(const Config &config)
        : glm::v4::ConditionalGeneration(config, MODEL_TYPE_CODEGEEX4)
    {
    }

    // Tool calling seems to be wrapped by ```json ... ```,
    // which shall be handled by, such as Python bindings
    ChunkInterceptor *get_interceptor(void) override { return nullptr; }
};
}