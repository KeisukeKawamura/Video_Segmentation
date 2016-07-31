/******************************************/
/*                                        */
/*   コピー禁止クラスの基底クラスの定義   */
/*                                        */
/******************************************/


#ifndef NONCOPYABLE_H__
#define NONCOPYABLE_H__


namespace kn
{
	// コピー禁止クラスの基底クラス
	// このクラスを private で継承することにより，派生クラスにコピー禁止属性を持たせることができる
	class Noncopyable
	{
	private:
		void operator =(const Noncopyable& src); // 代入演算子を private 宣言することにより代入演算を禁止
		Noncopyable(const Noncopyable& src);     // コピーコンストラクタを private 宣言することによりコピーインスタンスの生成を禁止

		// ... 以上の性質は派生クラスに継承される

	protected:
		// コンストラクタ・デストラクタを protected 宣言することにより，
		// このクラス自体のインスタンスの生成を禁止する（派生クラスからの生成を除く）
		Noncopyable() {}
		~Noncopyable() {}
	};
}


#endif
