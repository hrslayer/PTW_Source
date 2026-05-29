// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Shop/PTWItemInfoWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "System/Shop/PTWShopSubsystem.h"
#include "Inventory/PTWItemDefinition.h"

void UPTWItemInfoWidget::SetItemID(FName InItemID)
{
	ItemID = InItemID;
	Refresh();
}

void UPTWItemInfoWidget::Refresh()
{
	if (ItemID.IsNone()) return;

	if (UPTWShopSubsystem* ShopSys = GetWorld()->GetSubsystem<UPTWShopSubsystem>())
	{
		const FShopItemRow* Data = ShopSys->GetShopItemData(ItemID);
		if (!Data) return;

		/* 아이템 이름 */
		if (Text_ItemName)
		{
			Text_ItemName->SetText(Data->DisplayName);
		}

		/* 카테고리 출력 추가 */
		if (Text_Category)
		{
			// Enum 값을 문자열/텍스트로 변환
			const UEnum* CategoryEnum = StaticEnum<EShopCategory>();
			FText CategoryText = CategoryEnum->GetDisplayNameTextByValue((int64)Data->Category);
			Text_Category->SetText(CategoryText);
		}

		/* ItemType 출력 (ItemDefinition 기반) */
		if (Text_ItemType)
		{
			// ItemDefinition이 존재하는 경우에만 처리
			if (UPTWItemDefinition* ItemDef = Data->ItemDefinition.LoadSynchronous())
			{
				if (Text_ItemType)
				{
					const UEnum* TypeEnum = StaticEnum<EItemType>();
					FText TypeText = TypeEnum->GetDisplayNameTextByValue((int64)ItemDef->ItemType);

					Text_ItemType->SetText(TypeText);
					Text_ItemType->SetVisibility(ESlateVisibility::Visible);
				}

				if (Image_ItemIcon)
				{
					UTexture2D* IconTexture = ItemDef->ItemIcon.LoadSynchronous();
					if (IconTexture)
					{
						Image_ItemIcon->SetBrushFromTexture(IconTexture);
					}
				}
			}
			else
			{
				Text_ItemType->SetVisibility(ESlateVisibility::Collapsed);
			}
		}

		/* 아이템 가격 */
		const int32 BasePrice = Data->BasePrice;
		const int32 FinalPrice = ShopSys->GetItemPrice(ItemID);

		if (Text_Price)
		{
			static FSlateColor DefaultPriceColor = Text_Price->GetColorAndOpacity();

			Text_Price->SetText(FText::Format(NSLOCTEXT("Shop", "Price", "{0} G"), FinalPrice));

			if (FinalPrice > BasePrice)
			{
				// 가격 상승: 빨간색
				Text_Price->SetColorAndOpacity(FSlateColor(FLinearColor::Red));
			}
			else if (FinalPrice < BasePrice)
			{
				// 가격 하락: 초록색
				Text_Price->SetColorAndOpacity(FSlateColor(FLinearColor::Green));
			}
			else
			{
				// 변동 없음: 캐싱해둔 에디터 기본 색상으로 복구
				Text_Price->SetColorAndOpacity(DefaultPriceColor);
			}
		}

		if (Text_PriceDelta)
		{
			if (FinalPrice == BasePrice)
			{
				Text_PriceDelta->SetVisibility(ESlateVisibility::Collapsed);
			}
			else
			{
				Text_PriceDelta->SetText(FText::Format(NSLOCTEXT("Shop", "BasePrice", "{0} G"), BasePrice));
				Text_PriceDelta->SetVisibility(ESlateVisibility::Visible);
			}
		}


		/* 아이템 설명 */
		if (Text_Description)
		{
			Text_Description->SetText(Data->Description);
		}
	}
}
