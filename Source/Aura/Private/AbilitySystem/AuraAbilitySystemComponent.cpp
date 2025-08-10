// Copyright LeeSeungwon


#include "AbilitySystem/AuraAbilitySystemComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AuraGameplayTags.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "AbilitySystem/Abilities/AuraGameplayAbility.h"
#include "AbilitySystem/Data/AbilityInfo.h"
#include "Aura/AuraLogChannels.h"
#include "Game/LoadScreenSaveGame.h"
#include "Interaction/PlayerInterface.h"

void UAuraAbilitySystemComponent::AbilityActorInfoSet()
{
	OnGameplayEffectAppliedDelegateToSelf.AddUObject(this, &UAuraAbilitySystemComponent::ClientEffectApplied);//Dynamic아니므로 Adduboject)
	//OnGameplayEffectAppliedDelegateToSelf: 서버에서 GE가 자기 자신에게 적용될 때마다 호출.Instant, Duration GE가 포함된다.->아무 설정 없이는 클라이언트 측면에서 메시지 위젯이 안 뜨는 문제 발생
	/*
	 * EffectApplied 이 함수를 클라이언트 RPC로 만들면 서버에서 호출되고 클라이언트에서도 호출될 것이다. 클라이언트 RPC는 서버에서 호출되고 클라이언트에서 실행되도록 만들어진다.
	 * 서버에 있는 ASC(호스팅 하는 플레이어)에서 호출되면 클라이언트 RPC는 서버에서 호출된다. 그리고 클라이언트 쪽으로 레플리케이트 되지 않는다(클라이언트가 서버에 있으므로).
	 *
	 * ClientEffectApplied로 만들어서 UFUNCTION붙이고 설정 해 놓으면 서버에서 브로드캐스트 할 때 서버에서 호출되고 클라이언트에서 실행된다.
	 */
}

void UAuraAbilitySystemComponent::AddCharacterAbilitiesFromSaveData(ULoadScreenSaveGame* SaveData)
{
	
	for (const FSavedAbility Data: SaveData->SavedAbilities)
	{
		const TSubclassOf<UGameplayAbility> LoadedAbilityClass = Data.GameplayAbility;

		FGameplayAbilitySpec LoadedAbilitySpec = FGameplayAbilitySpec(LoadedAbilityClass, Data.AbilityLevel);

		LoadedAbilitySpec.DynamicAbilityTags.AddTag(Data.AbilitySlot);
		LoadedAbilitySpec.DynamicAbilityTags.AddTag(Data.AbilityStatus);
		
		if (Data.AbilityType == FAuraGameplayTags::Get().Abilities_Type_Offensive)
		{
			GiveAbility(LoadedAbilitySpec);
		}
		else if (Data.AbilityType == FAuraGameplayTags::Get().Abilities_Type_Passive)
		{
			GiveAbility(LoadedAbilitySpec);
			if (Data.AbilityStatus.MatchesTagExact(FAuraGameplayTags::Get().Abilities_Status_Equipped))//장착된 어빌리티만 activate
			{
				TryActivateAbility(LoadedAbilitySpec.Handle);   
			}
			
		}
	}
	bStartupAbilitiesGiven = true;
	AbilitiesGivenDelegate.Broadcast();
}

void UAuraAbilitySystemComponent::AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupAbilities)
{
	for (const TSubclassOf<UGameplayAbility> AbilityClass : StartupAbilities)
	{
		//abilityspec만들고 넣어주기
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		if (const UAuraGameplayAbility* AuraAbility = Cast<UAuraGameplayAbility>(AbilitySpec.Ability))
		{
			AbilitySpec.DynamicAbilityTags.AddTag(AuraAbility->StartUpInputTag);//Enemy는 InputTag필요X
			/*
			 * 런타임에 InputTag를 바꾸고 싶다. -> 어떤 어빌리티를 왼쪽 마우스 버튼에 할당했다가 오른쪽마우스 버튼으로 할당하고 싶을 경우
			 * GameplayAbilitySpec은 Dynamic하게 추가되거나 삭제될 수 있는 태그 컨테이너를 가지고 있다.->이를 이용하려한다.
			 * ASC가 어빌리티를 게임 시작에 처음 넣으려하는 이 함수에서 StartupInputTag(각 AuraGameplayAbility에 있는 것)를 체크할 수 있고
			 * 그 어빌리티의 AbilitySpec에 넣을 수 있다. 어빌리티 스펙이 만들어진 후, 이 어빌리티 스펙을 가져와 DynamicAbilityTags에 넣을 수 있다.
			 */
			AbilitySpec.DynamicAbilityTags.AddTag(FAuraGameplayTags::Get().Abilities_Status_Equipped);
			GiveAbility(AbilitySpec); //const FGameplayAbilitySpec이어도 ㄱㅊ
			//GiveAbilityAndActivateOnce(AbilitySpec);  //const FGameplayAbilitySpec면 안 됨
		}
		
	}
	bStartupAbilitiesGiven = true;
	AbilitiesGivenDelegate.Broadcast();//일단 서버에서만 됨.
	
}

void UAuraAbilitySystemComponent::AddCharacterPassiveAbilities(
	const TArray<TSubclassOf<UGameplayAbility>>& StartupPassiveAbilities)
{
	for (const TSubclassOf<UGameplayAbility> AbilityClass : StartupPassiveAbilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		AbilitySpec.DynamicAbilityTags.AddTag(FAuraGameplayTags::Get().Abilities_Status_Equipped);//equip해주기, Activate되기 전에 add해줘야 함
		GiveAbilityAndActivateOnce(AbilitySpec); //passive라 장착하면 바로 activate
		
	}
}

void UAuraAbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;

	FScopedAbilityListLock ActiveScopedLock(*this);
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities()/*GameplayAbilitySpec TArray반환*/)
	{
		if (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag))
		{
			AbilitySpecInputPressed(AbilitySpec); 
			if (AbilitySpec.IsActive())
			{
				InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, AbilitySpec.Handle, AbilitySpec.ActivationInfo.GetActivationPredictionKey());
			}//InvokeReplicatedEvent: 이 명령어 없으면 wait input press이런 ability Task는 작동하지 않는다.
		}
	}
}

void UAuraAbilitySystemComponent::AbilityInputTagHeld(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;
	FScopedAbilityListLock ActiveScopedLock(*this);
	//InputTag와 관련된 어빌리티 중 Active가능한 어빌리티 찾기
	for(FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities()/*GameplayAbilitySpec TArray반환*/)
	{
		if (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag))
		{
			AbilitySpecInputPressed(AbilitySpec); //현재 어빌리티가 눌려지고 있는지 어빌리티에게 알려주는 함수
			if (!AbilitySpec.IsActive())//Active안하고 있다면
			{
				TryActivateAbility(AbilitySpec.Handle);//Activate
			}
		}	
	}
}

void UAuraAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
	
	if (!InputTag.IsValid()) return;
	FScopedAbilityListLock ActiveScopedLock(*this);
	//InputTag와 관련된 어빌리티 중 Active가능한 어빌리티 찾기
	for(FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities()/*GameplayAbilitySpec TArray반환*/)
	{
		if (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag) && AbilitySpec.IsActive())
		{
			AbilitySpecInputReleased(AbilitySpec); //현재 어빌리티가 눌려지고 있는지 알려주는 함수
			//어빌리티가 손 땐다고 바로 그칠 어빌리티만 있는건 아니기에 그냥 더 이상 누르고 있지 않다는 것을 어빌리티에 알릴 함수 필요

			InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, AbilitySpec.Handle, AbilitySpec.ActivationInfo.GetActivationPredictionKey());
		}	
	}
}

void UAuraAbilitySystemComponent::ForEachAbility(const FForEachAbility& Delegate)
{
	FScopedAbilityListLock ActiveScopeLock(*this);//ActivatableAbilities: 루프 돌 때 이 Scope가 끝났을 때까지 Remove되려하거나 Add되려하는 어빌리티를 추적하면서 이 때문에 생기는 문제를 방지할 수 있다.
	for (const FGameplayAbilitySpec& AbilitySpec: GetActivatableAbilities())
	{
		if (!Delegate.ExecuteIfBound(AbilitySpec)) 
		{
			UE_LOG(LogAura, Error, TEXT("Failed to execute delegate in %hs"), __FUNCTION__);//__FUNCTION__ 함수 이름
		}
	}
}

FGameplayTag UAuraAbilitySystemComponent::GetAbilityTagFromSpec(const FGameplayAbilitySpec& AbilitySpec)
{
	if (AbilitySpec.Ability)
	{
		for (FGameplayTag Tag: AbilitySpec.Ability.Get()->AbilityTags)
		{
			if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Abilities"))))//ability만 뽑아내도록
			{
				return Tag;
			}
		}	
	}
	return FGameplayTag();
}

FGameplayTag UAuraAbilitySystemComponent::GetInputTagFromSpec(const FGameplayAbilitySpec& AbilitySpec)
{
	for (FGameplayTag Tag: AbilitySpec.DynamicAbilityTags)
	{
		if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("InputTag"))))//ability만 뽑아내도록
		{
			return Tag;
		}
	}
	return FGameplayTag();
}

FGameplayTag UAuraAbilitySystemComponent::GetStatusFromSpec(const FGameplayAbilitySpec& AbilitySpec)
{
	for (FGameplayTag StatusTag : AbilitySpec.DynamicAbilityTags)
	{
		if (StatusTag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Abilities.Status")))) //GameplayAbility는 하나의 Status태그만 가지게 되므로 이렇게만 하면 그 어빌리티의 상태만 고를 수 있음
		{
			return StatusTag;
		}
	}
	return FGameplayTag();
}

FGameplayTag UAuraAbilitySystemComponent::GetStatusFromAbilityTag(const FGameplayTag& AbilityTag)
{
	if (const FGameplayAbilitySpec* Spec = GetSpecFromAbilityTag(AbilityTag))
	{
		return GetStatusFromSpec(*Spec);
	}
	return FGameplayTag();
}

FGameplayTag UAuraAbilitySystemComponent::GetSlotFromAbilityTag(const FGameplayTag& AbilityTag)
{
	if (const FGameplayAbilitySpec* Spec = GetSpecFromAbilityTag(AbilityTag))
	{
		return GetInputTagFromSpec(*Spec);
	}
	return FGameplayTag();
}

bool UAuraAbilitySystemComponent::SlotIsEmpty(const FGameplayTag& Slot)
{
	FScopedAbilityListLock ActiveScopedLock(*this);
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilityHasSlot(AbilitySpec, Slot))
		{
			return false;
		}
	}
	return true;
}

bool UAuraAbilitySystemComponent::AbilityHasSlot(const FGameplayAbilitySpec& Spec, const FGameplayTag& Slot)
{
	return Spec.DynamicAbilityTags.HasTagExact(Slot);
}

FGameplayAbilitySpec* UAuraAbilitySystemComponent::GetSpecWithSlot(const FGameplayTag& Slot)
{
	FScopedAbilityListLock ActiveScopedLock(*this);
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.DynamicAbilityTags.HasTagExact(Slot))
		{
			return &AbilitySpec;
		}
	}
	return nullptr;
}

bool UAuraAbilitySystemComponent::IsPassiveAbility(const FGameplayAbilitySpec& Spec) const
{
	const UAbilityInfo* AbilityInfo = UAuraAbilitySystemLibrary::GetAbilityInfo(GetAvatarActor());
	const FGameplayTag AbilityTag = GetAbilityTagFromSpec(Spec);
	const FAuraAbilityInfo& Info = AbilityInfo->FindAbilityInfoForTag(AbilityTag);
	const FGameplayTag AbilityType = Info.AbilityType;

	return AbilityType.MatchesTagExact(FAuraGameplayTags::Get().Abilities_Type_Passive);
}

void UAuraAbilitySystemComponent::AssignSlotToAbility(FGameplayAbilitySpec& Spec, const FGameplayTag& Slot)
{
	ClearSlot(&Spec);
	Spec.DynamicAbilityTags.AddTag(Slot);
}

void UAuraAbilitySystemComponent::MulticastActivatePassiveEffect_Implementation(const FGameplayTag& AbilityTag, bool bActivate)
{
	ActivatePassiveEffect.Broadcast(AbilityTag, bActivate);
}

bool UAuraAbilitySystemComponent::AbilityHasAnySlot(const FGameplayAbilitySpec& Spec)
{
	return Spec.DynamicAbilityTags.HasTag(FGameplayTag::RequestGameplayTag(FName("InputTag")));
}

FGameplayAbilitySpec* UAuraAbilitySystemComponent::GetSpecFromAbilityTag(const FGameplayTag& AbilityTag)
{
	//루프 돌면서 변경, 추가, 삭제되면서 생기는 문제들을 겪지 않기위해 Lock
	FScopedAbilityListLock ActiveScopedLock(*this);
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		for (FGameplayTag Tag : AbilitySpec.Ability.Get()->AbilityTags)
		{
			if (Tag.MatchesTag(AbilityTag))
			{
				return &AbilitySpec;
			}
		}
	}
	return nullptr;
}

void UAuraAbilitySystemComponent::UpgradeAttribute(const FGameplayTag& AttributeTag)
{
	if (GetAvatarActor()->Implements<UPlayerInterface>())
	{
		if (IPlayerInterface::Execute_GetAttributePoints(GetAvatarActor()) > 0)
		{
			ServerUpgradeAttribute(AttributeTag);
		}
	}
}

void UAuraAbilitySystemComponent::ServerUpgradeAttribute_Implementation(const FGameplayTag& AttributeTag) //GA_Listenforevent 와 연관
{
	FGameplayEventData Payload;
	Payload.EventTag = AttributeTag;
	Payload.EventMagnitude = 1.f;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetAvatarActor(), AttributeTag, Payload); 

	if (GetAvatarActor()->Implements<UPlayerInterface>())
	{
		IPlayerInterface::Execute_AddToAttributePoints(GetAvatarActor(), -1 ); //한 번 쓸 때마다 1씩 깎임
	}
}

void UAuraAbilitySystemComponent::UpdateAbilityStatus(int32 Level) //레벨업할 때 사용되는 함수는 AuraCharacter에 있다. 거기에서 활용
{
	UAbilityInfo* AbilityInfo = UAuraAbilitySystemLibrary::GetAbilityInfo(GetAvatarActor());
	for (const FAuraAbilityInfo& Info : AbilityInfo->AbilityInformation)
	{
		if (!Info.AbilityTag.IsValid()) continue;
		if (Level < Info.LevelRequirement) continue;
		if (GetSpecFromAbilityTag(Info.AbilityTag) == nullptr) //ActivatableAbility에 없는 경우
		{
			FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(Info.Ability, 1);
			AbilitySpec.DynamicAbilityTags.AddTag(FAuraGameplayTags::Get().Abilities_Status_Eligible);//Eligible한 상태로 넣어준다.
			GiveAbility(AbilitySpec);//일단 Activate는 시키지 않고 가지기만 한다.
			MarkAbilitySpecDirty(AbilitySpec); //즉시 Replicate시키기 위해 사용하는 함수
			ClientUpdateAbilityStatus(Info.AbilityTag,FAuraGameplayTags::Get().Abilities_Status_Eligible, 1);//주어질 때 바로 전달
		}
	}
}

void UAuraAbilitySystemComponent::ServerSpendSpellPoint_Implementation(const FGameplayTag& AbilityTag)
{
	if (FGameplayAbilitySpec* AbilitySpec = GetSpecFromAbilityTag(AbilityTag))//Activatableability에 없을 수 있어서 null체크
	{
		if (GetAvatarActor()->Implements<UPlayerInterface>())
		{
			IPlayerInterface::Execute_AddToSpellPoints(GetAvatarActor(),-1);
		}
		const FAuraGameplayTags GameplayTags= FAuraGameplayTags::Get();
		//lock이면 Spend못함->나머지 3개 경우 체크
		FGameplayTag Status = GetStatusFromSpec(*AbilitySpec);
		if (Status.MatchesTagExact(GameplayTags.Abilities_Status_Eligible))
		{
			AbilitySpec->DynamicAbilityTags.RemoveTag(GameplayTags.Abilities_Status_Eligible);//GameplayTags.Abilities_Status_Eligible태그 제거하고 Unlocked로 갈아끼운다.
			AbilitySpec->DynamicAbilityTags.AddTag(GameplayTags.Abilities_Status_Unlocked);
			Status = GameplayTags.Abilities_Status_Unlocked;
		}
		else if (Status.MatchesTagExact(GameplayTags.Abilities_Status_Equipped) || Status.MatchesTagExact(GameplayTags.Abilities_Status_Unlocked))
		{
			AbilitySpec->Level++;
		}
		ClientUpdateAbilityStatus(AbilityTag, Status, AbilitySpec->Level);
		MarkAbilitySpecDirty(*AbilitySpec);
	}
}

void UAuraAbilitySystemComponent::ServerEquipAbility_Implementation(const FGameplayTag& AbilityTag,
	const FGameplayTag& Slot)//서버에서만 브로드캐스트. 특정 효과 같은 경우 Server, Client 다 볼 수 있도록 여기에서 브로드캐스트하지 않는다.// MulticastActivatePassiveEffect_Implementation에서 호출
{
	if (FGameplayAbilitySpec* AbilitySpec = GetSpecFromAbilityTag(AbilityTag))
	{
		const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::Get();
		const FGameplayTag& PrevSlot = GetInputTagFromSpec(*AbilitySpec);//바뀌어야 될 슬롯은 현재의 InputTag.
		const FGameplayTag& Status = GetStatusFromSpec(*AbilitySpec);

		const bool bStatusValid = Status == GameplayTags.Abilities_Status_Equipped || Status == GameplayTags.Abilities_Status_Unlocked;
		if (bStatusValid)
		{

			// Handle Activation/Deactivation for Passive Ability

			if (!SlotIsEmpty(Slot)) // There is an ability in this slot already. Deactivate and clear its slot.
			{
				FGameplayAbilitySpec* SpecWithSlot = GetSpecWithSlot(Slot);
				if (SpecWithSlot)
				{
					//Is that ability the same as this ability? if so, we can return early.
					if (AbilityTag.MatchesTagExact(GetAbilityTagFromSpec(*SpecWithSlot)))
					{
						ClientEquipAbility(AbilityTag, GameplayTags.Abilities_Status_Equipped, Slot, PrevSlot);//빠른 return하는 대신 클라에도 적용해야할것은 여기에도 해줘야한다.
						return;
					}

					if (IsPassiveAbility(*SpecWithSlot))
					{
						MulticastActivatePassiveEffect(GetAbilityTagFromSpec(*SpecWithSlot), false);
						DeactivatePassiveAbility.Broadcast(GetAbilityTagFromSpec(*SpecWithSlot));
					}
					ClearSlot(SpecWithSlot);
				}
			}

			if (!AbilityHasAnySlot(*AbilitySpec))//Ability doesn't yet have a slot(it's not active).
			{
				if (IsPassiveAbility(*AbilitySpec))
				{
					TryActivateAbility(AbilitySpec->Handle);
					MulticastActivatePassiveEffect(AbilityTag, true);
				}
				AbilitySpec->DynamicAbilityTags.RemoveTag(GetStatusFromSpec(*AbilitySpec));//삭제하고
				AbilitySpec->DynamicAbilityTags.AddTag(GameplayTags.Abilities_Status_Equipped);//equip
			}
			AssignSlotToAbility(*AbilitySpec, Slot);
			//Assign Slot To Ability
			////Remove this InputTag (slot) from any Ability that has it.
			//ClearAbilitiesOfSlot(Slot);
			////Clear this Ability's slot, just in case, it's a different slot
			//ClearSlot(AbilitySpec);
			////Now, assign this ability to this slot
			//AbilitySpec->DynamicAbilityTags.AddTag(Slot);
			//if (Status.MatchesTagExact(GameplayTags.Abilities_Status_Unlocked))
			//{
			//	AbilitySpec->DynamicAbilityTags.RemoveTag(GameplayTags.Abilities_Status_Unlocked);
			//	AbilitySpec->DynamicAbilityTags.AddTag(GameplayTags.Abilities_Status_Equipped);
			//}


			MarkAbilitySpecDirty(*AbilitySpec);
		}
		ClientEquipAbility(AbilityTag, GameplayTags.Abilities_Status_Equipped, Slot, PrevSlot);
	}
}

void UAuraAbilitySystemComponent::ClientEquipAbility_Implementation(const FGameplayTag& AbilityTag, const FGameplayTag& Status,
	const FGameplayTag& Slot, const FGameplayTag& PreviousSlot)
{
	AbilityEquipped.Broadcast(AbilityTag,Status, Slot, PreviousSlot);
}

bool UAuraAbilitySystemComponent::GetDescriptionByAbilityTag(const FGameplayTag& AbilityTag, FString& OutDescription,
                                                             FString& OutNextLevelDescription)
{
	if (const FGameplayAbilitySpec* AbilitySpec = GetSpecFromAbilityTag(AbilityTag))
	{
		if (UAuraGameplayAbility* AuraAbility = Cast<UAuraGameplayAbility>(AbilitySpec->Ability))
		{
			OutDescription = AuraAbility->GetDescription(AbilitySpec->Level);
			OutNextLevelDescription = AuraAbility->GetNextLevelDescription(AbilitySpec->Level + 1);
			return true;
		}
	}
	const UAbilityInfo* AbilityInfo = UAuraAbilitySystemLibrary::GetAbilityInfo(GetAvatarActor());
	if (!AbilityTag.IsValid()||AbilityTag.MatchesTagExact(FAuraGameplayTags::Get().Abilities_None))//AbilityMenu켜두고 레벨업될 때 이상한 디스크립션이 나옴-> 태그가 없거나 None일 때(보통 Deselect됐을 때) 아무런 디스크립션이 나오지 않도록 설정
	{
		OutDescription = FString();
	}
	else
	{
		OutDescription = UAuraGameplayAbility::GetLockedDescription(AbilityInfo->FindAbilityInfoForTag(AbilityTag).LevelRequirement);
	}
	OutNextLevelDescription = FString();
	return false;//Activatable한 Ability를 찾지 못한 경우
}

void UAuraAbilitySystemComponent::ClearSlot(FGameplayAbilitySpec* Spec)
{
	const FGameplayTag Slot = GetInputTagFromSpec(*Spec);
	Spec->DynamicAbilityTags.RemoveTag(Slot);
}

void UAuraAbilitySystemComponent::ClearAbilitiesOfSlot(const FGameplayTag& Slot)
{
	FScopedAbilityListLock ActiveScopeLock(*this);
	for (FGameplayAbilitySpec& Spec : GetActivatableAbilities())
	{
		if (AbilityHasSlot(&Spec, Slot))
		{
			ClearSlot(&Spec);
		}
	}
}

bool UAuraAbilitySystemComponent::AbilityHasSlot(FGameplayAbilitySpec* Spec, const FGameplayTag& Slot)
{
	for (FGameplayTag Tag: Spec->DynamicAbilityTags)
	{
		if (Tag.MatchesTagExact(Slot))
		{
			return true;
		}
	}
	return false;
}

void UAuraAbilitySystemComponent::OnRep_ActivateAbilities() 
{
	Super::OnRep_ActivateAbilities();

	if (!bStartupAbilitiesGiven) //서버에서 한 번만 할 것이고 리플리케이트 되지 않는다. //Activatable 한 Abilities가 레플리케이트 될 때마다 델리게이트 하지 않을 것이다. 클라이언트도 이제 스킬에 따라 SpellGlobe 위젯이 업데이트된다
	{
		bStartupAbilitiesGiven = true;
		AbilitiesGivenDelegate.Broadcast();	
	}
}



void UAuraAbilitySystemComponent::ClientUpdateAbilityStatus_Implementation(const FGameplayTag& AbilityTag,
                                                                           const FGameplayTag& StatusTag, int32 AbilityLevel)
{
	AbilityStatusChanged.Broadcast(AbilityTag, StatusTag, AbilityLevel);
}

void UAuraAbilitySystemComponent::ClientEffectApplied_Implementation(UAbilitySystemComponent* AbilitySystemComponent,
                                                                     const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle ActiveEffectHandle)
{
	//태그는 Tarray말고 GameplayTagContainer에 담는다.
	FGameplayTagContainer TagContainer;
	EffectSpec.GetAllAssetTags(TagContainer);

	EffectAssetTags.Broadcast(TagContainer); //EffectSpec에 있던 모든 태그를 가져온다. 이 때, 태그들은 OverlayWidgetController에서 사용
}
