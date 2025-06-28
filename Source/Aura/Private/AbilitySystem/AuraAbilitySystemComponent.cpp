// Copyright LeeSeungwon


#include "AbilitySystem/AuraAbilitySystemComponent.h"

#include "AuraGameplayTags.h"
#include "AbilitySystem/Abilities/AuraGameplayAbility.h"

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

void UAuraAbilitySystemComponent::AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupAbilities)
{
	for (const TSubclassOf<UGameplayAbility> AbilityClass:StartupAbilities)
	{
		//abilityspec만들고 넣어주기
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		if (const UAuraGameplayAbility* AuraAbility = Cast<UAuraGameplayAbility>(AbilitySpec.Ability))
		{
			AbilitySpec.DynamicAbilityTags.AddTag(AuraAbility->StartUpInputTag);
			/*
			 * 런타임에 InputTag를 바꾸고 싶다. -> 어떤 어빌리티를 왼쪽 마우스 버튼에 할당했다가 오른쪽마우스 버튼으로 할당하고 싶을 경우
			 * GameplayAbilitySpec은 Dynamic하게 추가되거나 삭제될 수 있는 태그 컨테이너를 가지고 있다.->이를 이용하려한다.
			 * ASC가 어빌리티를 게임 시작에 처음 넣으려하는 이 함수에서 StartupInputTag(각 AuraGameplayAbility에 있는 것)를 체크할 수 있고
			 * 그 어빌리티의 AbilitySpec에 넣을 수 있다. 어빌리티 스펙이 만들어진 후, 이 어빌리티 스펙을 가져와 DyanamicAbilityTags에 넣을 수 있다.
			 */

			GiveAbility(AbilitySpec); //const FGameplayAbilitySpec이어도 ㄱㅊ
			//GiveAbilityAndActivateOnce(AbilitySpec);  //const FGameplayAbilitySpec면 안 됨
		}
		
	}
}

void UAuraAbilitySystemComponent::AbilityInputTagHeld(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;
	
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
	
	//InputTag와 관련된 어빌리티 중 Active가능한 어빌리티 찾기
	for(FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities()/*GameplayAbilitySpec TArray반환*/)
	{
		if (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag))
		{
			AbilitySpecInputReleased(AbilitySpec); //현재 어빌리티가 눌려지고 있는지 알려주는 함수
			//어빌리티가 손 땐다고 바로 그칠 어빌리티만 있는건 아니기에 그냥 더 이상 누르고 있지 않다는 것을 어빌리티에 알릴 함수 필요
		}	
	}
}

void UAuraAbilitySystemComponent::ClientEffectApplied_Implementation(UAbilitySystemComponent* AbilitySystemComponent,
                                                const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle ActiveEffectHandle)
{
	//태그는 Tarray말고 GameplayTagContainer에 담는다.
	FGameplayTagContainer TagContainer;
	EffectSpec.GetAllAssetTags(TagContainer);

	EffectAssetTags.Broadcast(TagContainer);
}
