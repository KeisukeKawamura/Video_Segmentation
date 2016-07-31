/******************************************/
/*                                        */
/*   �R�s�[�֎~�N���X�̊��N���X�̒�`   */
/*                                        */
/******************************************/


#ifndef NONCOPYABLE_H__
#define NONCOPYABLE_H__


namespace kn
{
	// �R�s�[�֎~�N���X�̊��N���X
	// ���̃N���X�� private �Ōp�����邱�Ƃɂ��C�h���N���X�ɃR�s�[�֎~�������������邱�Ƃ��ł���
	class Noncopyable
	{
	private:
		void operator =(const Noncopyable& src); // ������Z�q�� private �錾���邱�Ƃɂ�������Z���֎~
		Noncopyable(const Noncopyable& src);     // �R�s�[�R���X�g���N�^�� private �錾���邱�Ƃɂ��R�s�[�C���X�^���X�̐������֎~

		// ... �ȏ�̐����͔h���N���X�Ɍp�������

	protected:
		// �R���X�g���N�^�E�f�X�g���N�^�� protected �錾���邱�Ƃɂ��C
		// ���̃N���X���̂̃C���X�^���X�̐������֎~����i�h���N���X����̐����������j
		Noncopyable() {}
		~Noncopyable() {}
	};
}


#endif
