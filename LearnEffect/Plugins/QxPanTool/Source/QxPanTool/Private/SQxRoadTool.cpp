// Fill out your copyright notice in the Description page of Project Settings.


#include "SQxRoadTool.h"

#include <stdexcept>

#include "SlateOptMacros.h"

#define LOCTEXT_NAMESPACE "QxRoadTool"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SQxRoadTool::Construct(const FArguments& InArgs)
{
	
}

void SQxRoadTool::OnSliderChanged(int32 Value)
{
	throw std::logic_error("Not implemented");
}

void SQxRoadTool::OnSliderSecLengthChanged(float Value)
{
	throw std::logic_error("Not implemented");
}

FReply SQxRoadTool::OnButtonClicked()
{
	throw std::logic_error("Not implemented");
}

FReply SQxRoadTool::OnResetSplineButtonClicked()
{
	throw std::logic_error("Not implemented");
}

void SQxRoadTool::GenerateSpline()
{
	throw std::logic_error("Not implemented");
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

#undef  LOCTEXT_NAMESPACE