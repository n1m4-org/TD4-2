#pragma once

#include "engine/gameobject/component/base/IActionComponent.h"
#include "jsonEditor/JsonEditableBase.h"
#include "math/Vector3.h"
#include "math/Vector4.h"
#include "engine/effects/particle/ParticleEffect.h"
#include <string>

namespace GameObjectComponent
{
	/**
	 * @brief トレイル（弾の跳ね返され時・ヒット時等）のエフェクト再生と制御を行う汎用コンポーネント。
	 */
	class TrailComponent : public IActionComponent
		, public JsonEditableBase
	{
	public:
		TrailComponent(const std::string& effectName = "smash");
		~TrailComponent();

		/**
		 * @brief コンポーネントの更新処理。
		 * @param owner このコンポーネントを所有するゲームオブジェクト
		 */
		void Update(GameObject* owner) override;

		/**
		 * @brief スマッシュエフェクトを再生する。
		 * @param position エフェクトの再生位置（指定しない場合は所有オブジェクトの位置）
		 */
		void Play(GameObject* owner = nullptr);

		/**
		 * @brief 現在再生中のエフェクトを個別停止する。
		 */
		void Stop();

		/**
		 * @brief エフェクトが再生中か確認する。
		 * @return 再生中ならtrue
		 */
		bool IsPlaying() const;

		/**
		 * @brief 再生するエフェクト名をセットする。
		 * @param effectName エフェクト名
		 */
		void SetEffectName(const std::string& effectName) { effectName_ = effectName; }

		/**
		 * @brief 再生するエフェクト名を取得する。
		 * @return エフェクト名
		 */
		const std::string& GetEffectName() const { return effectName_; }

		/**
		 * @brief レンダラーのカラー（ティントカラー）を設定する。
		 * @param color カラー（RGBA）
		 */
		void SetColor(const Vector4& color);

	private:
		// 再生するエフェクト識別名
		std::string effectName_ = "smash";

		// 現在再生中のエフェクトインスタンス（ParticleManagerが所有）
		ParticleEffect* currentEffect_ = nullptr;

	};
} // namespace GameObjectComponent
