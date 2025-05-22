#include "Direct3D.h"
#include "Global.h"
#include "Transform.h"
#include <d3dcompiler.h>

// ��ʂ̏����Ɋւ��鏈��
namespace Direct3D
{
    // �y�X���b�v�`�F�[���z
    // ��p����2���p�ӂ��Ēu���A�Е�����ʂɉf���Ă���Ԃɂ�������ɕ`��B
    // 1�t���[�����̊G���o���オ�������p���������B����ɂ���ʂ�������Ȃ�
    // ���̕ӂ������ǂ�̂��X���b�v�`�F�[��
    IDXGISwapChain* pSwapChain_ = nullptr;

    // �y�����_�[�^�[�Q�b�g�r���[�z
    // �`�悵�������̂ƁA�`���i��ł�����p���j�̋��n�����������
    ID3D11RenderTargetView* pRenderTargetView_ = nullptr;

    // �y�f�v�X�X�e���V���z
    // Z�o�b�t�@�@��p���āA�`���i��ł�����p���j�̋��n�����������
    ID3D11Texture2D* pDepthStencil;

    // �y�f�v�X�X�e���V���r���[�z
    // �f�v�X�X�e���V���̏����V�F�[�_�[�ɓn�����߂̂���
    ID3D11DepthStencilView* pDepthStencilView;
    ID3D11DepthStencilState* pDepthStencilState[BLEND_MAX];

    // �y�u�����h�X�e�[�g�z
    // �������̂��̂��ǂ̂悤�ɕ\�����邩
    ID3D11BlendState* pBlendState[BLEND_MAX];

    bool isDrawCollision_ = true; // �R���W������\�����邩
    bool _isLighting = false;     // ���C�e�B���O���邩

    // extern�錾�����ϐ��̏�����
    ID3D11Device* pDevice_ = nullptr;
    ID3D11DeviceContext* pContext_ = nullptr;
    SHADER_BUNDLE shaderBundle[SHADER_MAX] = { 0 };
    int screenWidth_ = 0;
    int screenHeight_ = 0;

    HRESULT Direct3D::Initialize(HWND hWnd, int screenWidth, int screenHeight)
    {
        // ���낢�돀�����邽�߂̐ݒ�
        // ���낢��Ȑݒ荀�ڂ��܂Ƃ߂��\����
        DXGI_SWAP_CHAIN_DESC scDesc;

        // �Ƃ肠�����S��0
        ZeroMemory(&scDesc, sizeof(scDesc));

        // �`���̃t�H�[�}�b�g
        scDesc.BufferDesc.Width = screenWidth; // ��ʕ�
        scDesc.BufferDesc.Height = screenHeight; // ��ʍ���
        scDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // ���F�g���邩

        // FPS(1/60�b��1��j
        scDesc.BufferDesc.RefreshRate.Numerator = 60;
        scDesc.BufferDesc.RefreshRate.Denominator = 1;

        // ���̑�
        scDesc.Windowed = TRUE; // �E�B���h�E���[�h���t���X�N���[����
        scDesc.OutputWindow = hWnd;  // �E�B���h�E�n���h��
        scDesc.BufferCount = 1; // ����ʂ̖���
        scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // ��ʂɕ`�悷�邽�߂Ɏg��
        scDesc.SampleDesc.Count = 1; // MSAA�i�A���`�G�C���A�X�j�̐ݒ�
        scDesc.SampleDesc.Quality = 0; // "

        // ��L�ݒ�����Ƀf�o�C�X�A�R���e�L�X�g�A�X���b�v�`�F�C�����쐬
        D3D_FEATURE_LEVEL level; 
        HRESULT hr;
        hr = D3D11CreateDeviceAndSwapChain(
            nullptr,					// �ǂ̃r�f�I�A�_�v�^���g�p���邩�H����Ȃ��nullptr��
            D3D_DRIVER_TYPE_HARDWARE,	// �h���C�o�̃^�C�v��n���B����ȊO�͊�{�I�Ƀ\�t�g�E�F�A�����ŁA�ǂ����Ă��Ƃ�������f�o�O�p�ɗp����ׂ�.
            nullptr,					// ��L��D3D_DRIVER_TYPE_SOFTWARE�ɐݒ肵���ۂɁA���̏������s��DLL�̃n���h����n���B����ȊO���w�肵�Ă���ۂɂ͕K��nullptr��n��.
            0,							// ���炩�̃t���O���w�肷��B�i�f�o�b�O����D3D11_CREATE_DEVICE_DEBUG�H�j
            nullptr,					// �f�o�C�X�A�R���e�L�X�g�̃��x����ݒ�Bnullptr�ɂ��Ƃ��Ή\�Ȍ��荂�����x���ɂ��Ă����
            0,							// ��̈����Ń��x�������w�肵����
            D3D11_SDK_VERSION,			// SDK�̃o�[�W�����B�K�����̒l
            &scDesc,					// ��ł��낢��ݒ肵���\����
            &pSwapChain_,				// ������������SwapChain�̃A�h���X���Ԃ��Ă���
            &pDevice_,					// ������������Device�A�h���X���Ԃ��Ă���
            &level,						// ������������Device�AContext�̃��x�����Ԃ��Ă���
            &pContext_);				// ������������Context�̃A�h���X���Ԃ��Ă���

        //���s������I��
        if (FAILED(hr))	return hr;

        ///////////////////////////�`��̂��߂̏���///////////////////////////////
        //�X���b�v�`�F�[������o�b�N�o�b�t�@���擾�i�o�b�N�o�b�t�@ �� ����� �� �`���j
        ID3D11Texture2D* pBackBuffer;
        hr = pSwapChain_->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

        //���s������I��
        if (FAILED(hr))	return hr;

        //�����_�[�^�[�Q�b�g�r���[���쐬
        hr = pDevice_->CreateRenderTargetView(pBackBuffer, NULL, &pRenderTargetView_);

        //���s������I��
        if (FAILED(hr))	return hr;

        //�ꎞ�I�Ƀo�b�N�o�b�t�@���擾���������Ȃ̂ŁA���
        pBackBuffer->Release();

        // �r���[�|�[�g�̐ݒ�
        // �����_�����O���ʂ�\������͈�
        D3D11_VIEWPORT vp;
        vp.Width = (float)screenWidth;   // ��
        vp.Height = (float)screenHeight; // ����
        vp.MinDepth = 0.0f; // ��O
        vp.MaxDepth = 1.0f; // ��
        vp.TopLeftX = 0;    // ��
        vp.TopLeftY = 0;    // ��

        // �e�p�^�[���̃V�F�[�_�[�Z�b�g����
        InitShaderBundle();
        Direct3D::SetShader(Direct3D::SHADER_3D);

        // �[�x�X�e���V���r���[�̍쐬
        // �[�x�o�b�t�@: �s�N�Z���̉��s�������L�^�i�B�ʏ����p�j
        // �X�e���V���o�b�t�@: �`�搧��p�}�X�N�i�A�E�g���C��/�e/�}�X�N�����j
        // �[�x�X�e���V���r���[: ���o�b�t�@�ւ�GPU�A�N�Z�X�����iDirect3D�p�j
        D3D11_TEXTURE2D_DESC descDepth;
        descDepth.Width = screenWidth;
        descDepth.Height = screenHeight;
        descDepth.MipLevels = 1;
        descDepth.ArraySize = 1;
        descDepth.Format = DXGI_FORMAT_D32_FLOAT;
        descDepth.SampleDesc.Count = 1;
        descDepth.SampleDesc.Quality = 0;
        descDepth.Usage = D3D11_USAGE_DEFAULT;
        descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        descDepth.CPUAccessFlags = 0;
        descDepth.MiscFlags = 0;
        pDevice_->CreateTexture2D(&descDepth, NULL, &pDepthStencil);
        pDevice_->CreateDepthStencilView(pDepthStencil, NULL, &pDepthStencilView);

        // �[�x�e�X�g���s���[�x�X�e���V���X�e�[�g�̍쐬
        {
            // �f�t�H���g
            D3D11_DEPTH_STENCIL_DESC desc = {};
            desc.DepthEnable = true;
            desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
            desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
            desc.StencilEnable = true;
            pDevice_->CreateDepthStencilState(&desc, &pDepthStencilState[BLEND_DEFAULT]);
            pContext_->OMSetDepthStencilState(pDepthStencilState[BLEND_DEFAULT], 0);

            // ���Z�����p�i�������݂Ȃ��j
            desc.StencilEnable = false;
            pDevice_->CreateDepthStencilState(&desc, &pDepthStencilState[BLEND_ADD]);
        }

        // �u�����h�X�e�[�g: �s�N�Z���̏o�͐F�ƕ`���̐F�̍������@���w��i����������Z�����ȂǂɎg�p�j
        {
            // �f�t�H���g
            D3D11_BLEND_DESC BlendDesc;
            ZeroMemory(&BlendDesc, sizeof(BlendDesc));
            BlendDesc.AlphaToCoverageEnable = FALSE;
            BlendDesc.IndependentBlendEnable = FALSE;
            BlendDesc.RenderTarget[0].BlendEnable = TRUE;

            BlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
            BlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;

            BlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
            BlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
            BlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
            BlendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
            BlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
            pDevice_->CreateBlendState(&BlendDesc, &pBlendState[BLEND_DEFAULT]);
            float blendFactor[4] = { D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, D3D11_BLEND_ZERO };
            pContext_->OMSetBlendState(pBlendState[BLEND_DEFAULT], blendFactor, 0xffffffff);

            // ���Z�����i�d�Ȃ�قǌ����Č��������)
            BlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
            BlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
            pDevice_->CreateBlendState(&BlendDesc, &pBlendState[BLEND_ADD]);
        }


        // �p�C�v���C���̍\�z
        // �f�[�^����ʂɕ`�悷�邽�߂̈�ʂ�̐ݒ�
        pContext_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // �f�[�^�̓��͎�ނ��w��
        pContext_->OMSetRenderTargets(1, &pRenderTargetView_, pDepthStencilView); // �`�����w��i����̓����_�[�^�[�Q�b�g�r���[����ĕ`�悷��j
        pContext_->RSSetViewports(1, &vp);                                        // �r���[�|�[�g�̃Z�b�g

        // �R���W�����\�����邩
        isDrawCollision_ = GetPrivateProfileInt("DEBUG", "ViewCollider", 0, ".\\setup.ini") != 0;

        screenWidth_ = screenWidth;
        screenHeight_ = screenHeight;

        return S_OK;
    }

    void InitShaderBundle()
    {
        DWORD vectorSize = sizeof(XMFLOAT3);
    }

    void Direct3D::SetShader(SHADER_TYPE type)
    {
    }

    void Direct3D::SetBlendMode(BLEND_MODE blendMode)
    {
    }

    void Direct3D::BeginDraw()
    {
    }

    void Direct3D::EndDraw()
    {
    }

    void Direct3D::Release()
    {
    }

    bool Direct3D::Intersect(XMFLOAT3& start, XMFLOAT3& direction, XMFLOAT3& v0, XMFLOAT3& v1, XMFLOAT3& v2, float* distance)
    {
        return false;
    }

    void Direct3D::SetDepthBafferWriteEnable(bool isWrite)
    {
    }

}