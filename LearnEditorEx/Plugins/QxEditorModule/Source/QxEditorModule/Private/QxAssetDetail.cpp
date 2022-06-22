// Fill out your copyright notice in the Description page of Project Settings.


#include "QxAssetDetail.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "PropertyCustomizationHelpers.h"
#include "Widgets/Input/SEditableTextBox.h"
#define LOCTEXT_NAMESPACE "QxAsset"


void FQxAssetDetail::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{


	
}


TSharedRef<IDetailCustomization> FQxAssetDetail::MakeInstance()
{
	return MakeShared<FQxAssetDetail>();
}

void FQxAssetDetail::CutomizeProperty(IDetailLayoutBuilder& DetailBuilder)
{
	TSharedRef<IPropertyHandle> IntegerHandle = DetailBuilder.GetProperty("Test2");
	IDetailCategoryBuilder& CustCategory = DetailBuilder.EditCategory(FName("CustCategory"), LOCTEXT("CustCategory", "Custom Category"), ECategoryPriority::Important);
	CustCategory.AddCustomRow(LOCTEXT("SearchText", "SearchText")).WholeRowContent().HAlign(HAlign_Left)
		[
			SNew(SBox).MinDesiredWidth(200.f)
			[
				SNew(SProperty, IntegerHandle).CustomWidget()
				[
					SNew(SHorizontalBox) + SHorizontalBox::Slot()
					[
						SNew(SCheckBox)
					] + SHorizontalBox::Slot()
					[
						SNew(SEditableTextBox)
					]
				]
			]
		];
}

void FQxAssetDetail::CutomizeProperty2(IDetailLayoutBuilder& DetailBuilder)
{
	TSharedRef<IPropertyHandle> IntegerHandle = DetailBuilder.GetProperty("IntegerVar");
	IDetailCategoryBuilder& CustCategory = DetailBuilder.EditCategory(FName("CustCategory"), LOCTEXT("CustCategory", "Custom Category"), ECategoryPriority::Important);
	CustCategory.AddCustomRow(LOCTEXT("SearchText", "SearchText")).NameContent()
		[
			SNew(STextBlock).Text(IntegerHandle->GetPropertyDisplayName())
		].ValueContent()
		[
			SNew(SHorizontalBox)+SHorizontalBox::Slot()
			[
				SNew(SCheckBox)
			]+ SHorizontalBox::Slot()
			[
				SNew(SEditableTextBox)
			]
		];
}

void FQxAssetDetail::GetPropertyAndAddToCategory(IDetailLayoutBuilder& DetailBuilder)
{
	//获取属性句柄，这里获取的就是上边的IntegerVar变量的句柄
	TSharedRef<IPropertyHandle> IntegerHandle = DetailBuilder.GetProperty("Test2");
	//获取目录构造器
	IDetailCategoryBuilder& CustCategory = DetailBuilder.EditCategory(FName("CustCategory"), LOCTEXT("CustCategory", "Custom Category"), ECategoryPriority::Important);
	//在目录中添加一行，并将
	CustCategory.AddCustomRow(LOCTEXT("SearchText", "SearchText")).WholeRowContent()
		[	//可以使用SProperty通过句柄直接创建一个默认的UI
			SNew(SProperty, IntegerHandle)
		];
	//也可以通过下面的两个方法添加到目录，不过这个就和UPROPERTY宏的作用一样了
	//第一个参数是属性句柄，第二个是查询关键词，第三个是是否是高级选项（上面演示过了）
	DetailBuilder.AddCustomRowToCategory(IntegerHandle, LOCTEXT("SearchInt", "SearchInt"), false);
	DetailBuilder.AddPropertyToCategory(IntegerHandle);
}

void FQxAssetDetail::Test1(IDetailLayoutBuilder& DetailBuilder)
{
	//返回指定目录的构造对象。第一个参数用于查找指定目录（必要时也可以作为目录名），第二个参数为显示目录名（可选），第三个参数为优先级，下面设置的是Important，所以会在顶层显示此目录。
	IDetailCategoryBuilder& CustCategory =
		DetailBuilder.EditCategory(
			FName("CustomCategory"),
			LOCTEXT("CustomCategory", "Custom Category"),
			ECategoryPriority::Important);
	 
	// DetailBuilder.EditCategory(FName("CustCategory"), LOCTEXT("CustCategory", "Custom Category"), ECategoryPriority::Important);
	//为此目录添加一行记录。第一个参数为用于过滤（搜索）的文本，第二个参数是是否为高级项。
	CustCategory.AddCustomRow(LOCTEXT("CustRow", "CustRow"), false).NameContent()//NameContent是属性名的显示内容
		[
			SNew(STextBlock).Text(LOCTEXT("TextBlock", "TextBlock"))
		].ValueContent()
		[	//ValueContent是值的显示内容。如果不想区分Key和Value分割的话，可以直接使用WholeRowContent，下面有效果。
			SNew(SVerticalBox)+SVerticalBox::Slot().HAlign(HAlign_Fill).AutoHeight()
			[
				SNew(SCheckBox)
			]+ SVerticalBox::Slot().HAlign(HAlign_Fill).AutoHeight()
			[
				SNew(SEditableTextBox)
			]
		];
}


#undef LOCTEXT_NAMESPACE
