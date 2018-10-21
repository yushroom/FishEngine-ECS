#include "CommandContext.hpp"

void ContextManager::DestroyAllContexts()
{
    for (auto& c : sm_ContextPool)
        c.clear();
}

CommandContext* ContextManager::AllocateContext(CommandListType type)
{
    std::lock_guard<std::mutex> lockGuard(sm_ContextAllocationMutex);

    int idx = static_cast<int>(type);
    auto& avaliableContexts = sm_AvaliableContexts[idx];

    CommandContext* ret = nullptr;
    if (avaliableContexts.empty())
    {
        ret = new CommandContext(type);
        sm_ContextPool[idx].emplace_back(ret);
        ret->Initialize();
    }
    else
    {
        ret = avaliableContexts.front();
        avaliableContexts.pop();
        ret->Reset();
    }
    assert(ret != nullptr);
    assert(ret->m_Type == type);

    return ret;
}

#define g_ContextManager ContextManager::GetInstance()

CommandContext& CommandContext::Begin(const std::string& ID)
{
    CommandContext* newContext = g_ContextManager.AllocateContext(CommandListType::Direct);
    newContext->SetID(ID);
    // if (ID.length() > 0)
    return *newContext;
}

// Flush existing commands to the GPU but keep the context alive
uint64_t CommandContext::Flush(bool waitForCompletion/* = false*/)
{
    return 0;
}

// Flush existing commands and release the current context
uint64_t CommandContext::Finish(bool waitForCompletion/* = false*/)
{
    return 0;
}

CommandContext::CommandContext(CommandListType type) :
    m_Type(type)
{

}

CommandContext::~CommandContext()
{

}

void CommandContext::Initialize()
{
    //g_ContextManager
}

void CommandContext::Reset()
{

}