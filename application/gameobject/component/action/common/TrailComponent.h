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
		 * 一度指定すると、以後エフェクトが再生し直された際にもこの色を維持する。
		 * 一度も呼ばれない場合はプリセット（JSON）側の色をそのまま使用する。
		 * @param color カラー（RGBA）
		 */
		void SetColor(const Vector4& color);

	private:
		/**
		 * @brief SetColorで指定された基準色を、再生中のエフェクトへ適用する。
		 * @details ParticleManagerのプールは前回再生時のティントカラーを保持したまま
		 *          使い回されることがあるため、SetColorで明示的に色指定した場合のみ、
		 *          Play()で新しいエフェクトを取得するたびに呼び出して前の所有者の色を上書きする。
		 */
		void ApplyBaseColor();

		// 再生するエフェクト識別名
		std::string effectName_ = "smash";

		// 現在再生中のエフェクトインスタンス（ParticleManagerが所有）
		ParticleEffect* currentEffect_ = nullptr;

		// 再生開始時に適用する基準色（SetColorで更新される）
		Vector4 baseColor_ = {1.0f, 1.0f, 1.0f, 1.0f};

		// SetColorが一度でも呼ばれたか（falseのままならプリセット色を上書きしない）
		bool hasCustomColor_ = false;

	};
} // namespace GameObjectComponent
