// Fill out your copyright notice in the Description page of Project Settings.


#include "ColorStructDetail.h"

#include "DetailWidgetRow.h"
#include "QxCustomObject.h"
#define LOCTEXT_NAMESPACE "FQxEditorModuleModule"
void FColorStructDetail::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow,
                                         IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	HeaderRow.NameContent()	//属性的Name样式
		[
			PropertyHandle->CreatePropertyNameWidget(LOCTEXT("UseRed", "Use Red"))
		].ValueContent()		//属性的Value样式
		[	//当CheckBox状态发生改变时，触发OnCheckBoxStateChanged方法
			SAssignNew(CheckBox, SCheckBox).OnCheckStateChanged_Raw(this, &FColorStructDetail::OnCheckBoxStateChanged)
		];
}

void FColorStructDetail::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle,
	IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	//获取结构体的两个属性句柄
	ColorHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FColorStruct, Color));
	BoolHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FColorStruct, bRed));
	if (BoolHandle.IsValid() && CheckBox.IsValid())
	{	//当显示属性内容时，初始化样式内容，上面的初始化样式，此方法初始化值
		bool bRed;
		BoolHandle->GetValue(bRed);
		if (bRed)
			CheckBox->SetIsChecked(ECheckBoxState::Checked);
		else
			CheckBox->SetIsChecked(ECheckBoxState::Unchecked);
	}
}

void FColorStructDetail::OnCheckBoxStateChanged(ECheckBoxState State)
{
	if (ColorHandle.IsValid() && BoolHandle.IsValid())		//当CheckBox状态发生改变，且结构体属性句柄有效，将修改其内容
		{
		switch (State)
		{
		case ECheckBoxState::Unchecked:				//取消选中
			ColorHandle->SetValue(FString("Not Red"));	//设置Color的内容为“Not Red”,SetValue的参数必须要与属性参数一致，否则会赋值失败！必要时，可以通过UProperty通过内存地址修改值
			BoolHandle->SetValue(false);				//记录Checkbox状态的变量修改
			break;
		case ECheckBoxState::Checked:
			ColorHandle->SetValue(FString("Red"));
			BoolHandle->SetValue(true);
			break;
		case ECheckBoxState::Undetermined:
			ColorHandle->SetValue(FString("Red"));
			BoolHandle->SetValue(true);
			break;
		default:
			break;
		}
		}
}

TSharedRef<IPropertyTypeCustomization> FColorStructDetail::MakeInstance()
{
	return MakeShared<FColorStructDetail>();
}

#undef LOCTEXT_NAMESPACE