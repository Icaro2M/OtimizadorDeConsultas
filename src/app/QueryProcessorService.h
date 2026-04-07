#pragma once

#include "QueryProcessingResult.h"

#include <string>
#include <vector>

class ExecutionPlan;
class ExecutionNode;
class MetadataCatalog;

class QueryProcessorService final
{
public:
    QueryProcessorService(const MetadataCatalog& metadataCatalog);

    QueryProcessingResult process(const std::string& sql) const;

private:
    PlanNodeView buildPlanNodeView(const ExecutionNode& node) const;
    void fillExecutionOrder(const ExecutionNode& node, std::vector<std::string>& executionOrder) const;
    std::string buildRelationalAlgebra(const ExecutionPlan& plan) const;

private:
    const MetadataCatalog& m_MetadataCatalog;
};