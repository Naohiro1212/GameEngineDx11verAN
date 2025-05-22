#include "Direct3D.h"
#include "Global.h"
#include "Transform.h"
#include <d3dcompiler.h>

// 画面の処理に関する処理
namespace Direct3D
{
    // 【スワップチェーン】
    // 画用紙を2枚用意して置き、片方を画面に映している間にもう一方に描画。
    // 1フレーム分の絵が出来上がったら画用紙を交換。これにより画面がちらつかない
    // その辺をつかさどるのがスワップチェーン
    IDXGISwapChain* pSwapChain_ = nullptr;

    // 【レンダーターゲットビュー】
    // 描画したいものと、描画先（上でいう画用紙）の橋渡しをするもの
    ID3D11RenderTargetView* pRenderTargetView_ = nullptr;

    // 【デプスステンシル】
    // Zバッファ法を用いて、描画先（上でいう画用紙）の橋渡しをするもの
    ID3D11Texture2D* pDepthStencil;

    // 【デプスステンシルビュー】
    // デプスステンシルの情報をシェーダーに渡すためのもの
    ID3D11DepthStencilView* pDepthStencilView;
    ID3D11DepthStencilState* pDepthStencilState[BLEND_MAX];

    // 【ブレンドステート】
    // 半透明のものをどのように表現するか
    ID3D11BlendState* pBlendState[BLEND_MAX];

    bool isDrawCollision_ = true; // コリジョンを表示するか
    bool _isLighting = false;     // ライティングするか

    // extern宣言した変数の初期化
    ID3D11Device* pDevice_ = nullptr;
    ID3D11DeviceContext* pContext_ = nullptr;
    SHADER_BUNDLE shaderBundle[SHADER_MAX] = { 0 };
    int screenWidth_ = 0;
    int screenHeight_ = 0;

    HRESULT Direct3D::Initialize(HWND hWnd, int screenWidth, int screenHeight)
    {
        // いろいろ準備するための設定
        // いろいろな設定項目をまとめた構造体
        DXGI_SWAP_CHAIN_DESC scDesc;

        // とりあえず全部0
        ZeroMemory(&scDesc, sizeof(scDesc));

        // 描画先のフォーマット
        scDesc.BufferDesc.Width = screenWidth; // 画面幅
        scDesc.BufferDesc.Height = screenHeight; // 画面高さ
        scDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // 何色使えるか

        // FPS(1/60秒に1回）
        scDesc.BufferDesc.RefreshRate.Numerator = 60;
        scDesc.BufferDesc.RefreshRate.Denominator = 1;

        // その他
        scDesc.Windowed = TRUE; // ウィンドウモードかフルスクリーンか
        scDesc.OutputWindow = hWnd;  // ウィンドウハンドル
        scDesc.BufferCount = 1; // 裏画面の枚数
        scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // 画面に描画するために使う
        scDesc.SampleDesc.Count = 1; // MSAA（アンチエイリアス）の設定
        scDesc.SampleDesc.Quality = 0; // "

        // 上記設定を元にデバイス、コンテキスト、スワップチェインを作成
        D3D_FEATURE_LEVEL level; 
        HRESULT hr;
        hr = D3D11CreateDeviceAndSwapChain(
            nullptr,					// どのビデオアダプタを使用するか？既定ならばnullptrで
            D3D_DRIVER_TYPE_HARDWARE,	// ドライバのタイプを渡す。これ以外は基本的にソフトウェア実装で、どうしてもという時やデバグ用に用いるべし.
            nullptr,					// 上記をD3D_DRIVER_TYPE_SOFTWAREに設定した際に、その処理を行うDLLのハンドルを渡す。それ以外を指定している際には必ずnullptrを渡す.
            0,							// 何らかのフラグを指定する。（デバッグ時はD3D11_CREATE_DEVICE_DEBUG？）
            nullptr,					// デバイス、コンテキストのレベルを設定。nullptrにしとけば可能な限り高いレベルにしてくれる
            0,							// 上の引数でレベルを何個指定したか
            D3D11_SDK_VERSION,			// SDKのバージョン。必ずこの値
            &scDesc,					// 上でいろいろ設定した構造体
            &pSwapChain_,				// 無事完成したSwapChainのアドレスが返ってくる
            &pDevice_,					// 無事完成したDeviceアドレスが返ってくる
            &level,						// 無事完成したDevice、Contextのレベルが返ってくる
            &pContext_);				// 無事完成したContextのアドレスが返ってくる

        //失敗したら終了
        if (FAILED(hr))	return hr;

        ///////////////////////////描画のための準備///////////////////////////////
        //スワップチェーンからバックバッファを取得（バックバッファ ＝ 裏画面 ＝ 描画先）
        ID3D11Texture2D* pBackBuffer;
        hr = pSwapChain_->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

        //失敗したら終了
        if (FAILED(hr))	return hr;

        //レンダーターゲットビューを作成
        hr = pDevice_->CreateRenderTargetView(pBackBuffer, NULL, &pRenderTargetView_);

        //失敗したら終了
        if (FAILED(hr))	return hr;

        //一時的にバックバッファを取得しただけなので、解放
        pBackBuffer->Release();

        // ビューポートの設定
        // レンダリング結果を表示する範囲
        D3D11_VIEWPORT vp;
        vp.Width = (float)screenWidth;   // 幅
        vp.Height = (float)screenHeight; // 高さ
        vp.MinDepth = 0.0f; // 手前
        vp.MaxDepth = 1.0f; // 奥
        vp.TopLeftX = 0;    // 左
        vp.TopLeftY = 0;    // 上

        // 各パターンのシェーダーセット準備
        InitShaderBundle();
        Direct3D::SetShader(Direct3D::SHADER_3D);

        // 深度ステンシルビューの作成
        // 深度バッファ: ピクセルの奥行き情報を記録（隠面消去用）
        // ステンシルバッファ: 描画制御用マスク（アウトライン/影/マスク処理）
        // 深度ステンシルビュー: 両バッファへのGPUアクセス窓口（Direct3D用）
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

        // 深度テストを行う深度ステンシルステートの作成
        {
            // デフォルト
            D3D11_DEPTH_STENCIL_DESC desc = {};
            desc.DepthEnable = true;
            desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
            desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
            desc.StencilEnable = true;
            pDevice_->CreateDepthStencilState(&desc, &pDepthStencilState[BLEND_DEFAULT]);
            pContext_->OMSetDepthStencilState(pDepthStencilState[BLEND_DEFAULT], 0);

            // 加算合成用（書き込みなし）
            desc.StencilEnable = false;
            pDevice_->CreateDepthStencilState(&desc, &pDepthStencilState[BLEND_ADD]);
        }

        // ブレンドステート: ピクセルの出力色と描画先の色の合成方法を指定（半透明や加算合成などに使用）
        {
            // デフォルト
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

            // 加算合成（重なるほど光って見える効果)
            BlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
            BlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
            pDevice_->CreateBlendState(&BlendDesc, &pBlendState[BLEND_ADD]);
        }


        // パイプラインの構築
        // データを画面に描画するための一通りの設定
        pContext_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // データの入力種類を指定
        pContext_->OMSetRenderTargets(1, &pRenderTargetView_, pDepthStencilView); // 描画先を指定（今後はレンダーターゲットビューを介して描画する）
        pContext_->RSSetViewports(1, &vp);                                        // ビューポートのセット

        // コリジョン表示するか
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