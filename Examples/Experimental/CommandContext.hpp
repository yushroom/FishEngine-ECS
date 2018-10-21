#include <vector>
#include <queue>
#include <mutex>
#include <memory>
#include <cassert>

// D3D12_COMMAND_LIST_TYPE
enum class CommandListType
{
    Direct  = 0,    // D3D12_COMMAND_LIST_TYPE_DIRECT
    Compute = 1,    // D3D12_COMMAND_LIST_TYPE_COMPUTE
    Copy    = 2     // D3D12_COMMAND_LIST_TYPE_COPY
};

class CommandContext;
class GraphicsContext;


struct NonCopyable
{
    NonCopyable() = default;
    NonCopyable(const NonCopyable&) = delete;
    NonCopyable & operator=(const NonCopyable&) = delete;
};


class ContextManager : NonCopyable
{
// public:
    ContextManager() = default;

public:
    static ContextManager& GetInstance()
    {
        static ContextManager inst;
        return inst;
    }

    CommandContext* AllocateContext(CommandListType type);
    void FreeContext(CommandContext*);
    void DestroyAllContexts();

private:
    std::vector<std::unique_ptr<CommandContext>> sm_ContextPool[4];
    std::queue<CommandContext*> sm_AvaliableContexts[4];
    std::mutex sm_ContextAllocationMutex;
};


class CommandContext : NonCopyable
{
    friend ContextManager;
private:
    CommandContext(CommandListType type);

    void Reset();

public:
    ~CommandContext();

    static void DestroyAllContexts();

    static CommandContext& Begin(const std::string& ID = "");

    void SetID(const std::string& ID) { m_ID = ID; }

    // Flush existing commands to the GPU but keep the context alive
    uint64_t Flush(bool waitForCompletion = false);

    // Flush existing commands and release the current context
    uint64_t Finish(bool waitForCompletion = false);

    // Prepare to render by reserving a command list and command allocator
    void Initialize();

    // GraphicsContext& GetGraphicsContext() {
    //     assert(m_Type != CommandListType::Compute, "Cannot convert async compute context to graphics");
    //     return reinterpret_cast<GraphicsContext&>(*this);
    // }

    // ComputeContext& GetComputeContext() {
    //     return reinterpret_cast<ComputeContext&>(*this);
    // }


protected:
    std::string m_ID;
    CommandListType m_Type;
};

class GraphicsContext : public CommandContext
{
public:
    static GraphicsContext& Begin(const std::string& ID = "");
};
