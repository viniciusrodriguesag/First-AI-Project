/*
 *	First AI Project
 *	12 / 10 / 2017
 *
 *	by Vinicius Rodrigues
 *
 *
 *
 */

#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <stdlib.h>
#include <vector>

#define SIGNAL(a) fabs(a)/a

struct D3DXVERTEX
{
	D3DXVECTOR4 pos;
	D3DCOLOR color;
	
	static const DWORD FVF; 
};
const DWORD D3DXVERTEX::FVF = D3DFVF_XYZRHW | D3DFVF_DIFFUSE;

float Vec2Distance( D3DXVECTOR2, D3DXVECTOR2 );
D3DXVECTOR2 AdjustVecInBox( D3DXVECTOR2, D3DXVECTOR4, bool pass = false );

class Germ
{
	public:
		enum GERM_STATE
		{
			WAITING, 
			EATING, 
			DRINKING, 
			REPRODUCING, 
			CRAWLING, 
			ROTATING, 
		};
		
		Germ( D3DXVECTOR2, bool, std::vector< Germ* >* );
		~Germ() {}
		
		void Mind( float );
		void Crawl( D3DXVECTOR2 );
		void Rotate( float );
		
		GERM_STATE GetState() { return GermState; }
		void SetState( GERM_STATE state ) { GermState = state; }
		D3DXVECTOR2 GetPosition() { return Position; }
		void SetPosition( D3DXVECTOR2 pos ) { Position = pos; }
		float GetRotation() { return Rotation; }
		void SetRotation( float rot ) { Rotation = rot; }
		bool GetSex() { return Sex; }
		float GetWaterNeed() { return WaterNeed; }
		float GetFoodNeed() { return FoodNeed; }
		float GetReproduceNeed() { return ReproduceNeed; }
		void SetReproduceNeed( float rn ) { ReproduceNeed = rn; }
		D3DXVECTOR2 GetInitialPosition() { return InitialPosition; }
		D3DXVECTOR2 GetObjective() { return FinalPosition; }
		float GetInitialRotation() { return InitialRotation; }
		float GetRotationObjective() { return FinalRotation; }
		Germ* GetFemale() { return Female; }	
		void SetFemaleNull() { Female = NULL; }
		void SetInPosition( bool b ) { InPosition = b; }
		bool GetInPosition() { return InPosition; }
		
		static const float speed = 40.0f;
		static const float rspeed = M_PI;
		static const float wneedspeed = 0.2f;
		static const float fneedspeed = 0.1f;
		static const float rneedspeed = 1.0f;
		static const float replacespeed = 2.0f;
		
		static const float needlimit = 10.0f;

	private:
		
		GERM_STATE GermState;
		D3DXVECTOR2 Position;
		float Rotation;
		bool Sex;
		float WaterNeed;
		float FoodNeed;
		float ReproduceNeed;
		bool InPosition;
		
		D3DXVECTOR2 InitialPosition;
		D3DXVECTOR2 FinalPosition;
		float InitialRotation;
		float FinalRotation;
		
		std::vector< Germ* > *ReproduceFemales;
		Germ* Female;
};

Germ::Germ( D3DXVECTOR2 pos, bool sex, std::vector< Germ* >* rf )
{
	Position = pos;
	Sex = sex;
	GermState = WAITING;
	ReproduceFemales = rf;
	Female = NULL;
	InPosition = false;
}

void Germ::Mind( float fTime )
{
	WaterNeed += wneedspeed * fTime;
	FoodNeed += fneedspeed * fTime;
	ReproduceNeed += rneedspeed * fTime;
	
	if( ReproduceNeed > needlimit )
	{
		if( Sex )
		{
			if( ReproduceFemales -> size() > 0 )
			{
				int fem = 0;
				float min_dist = Vec2Distance( (*ReproduceFemales)[ 0 ] -> GetPosition(), Position );
				for( int i = 0; i < ReproduceFemales -> size(); i++ )
				{
					int dist = Vec2Distance( (*ReproduceFemales)[ i ] -> GetPosition(), Position );
					if( dist < min_dist )
					{
						fem = i;
						min_dist = dist;
					}
				}
				Female = (*ReproduceFemales)[ fem ];
				(*ReproduceFemales)[ fem ] = (*ReproduceFemales)[ ReproduceFemales -> size() - 1 ];
				ReproduceFemales -> pop_back();
				D3DXVECTOR2 Dist = Female -> GetPosition() - Position;
				Rotate( atan( Dist.y / Dist.x ) - Rotation );
			}
		}
		else if( GermState!= REPRODUCING )
		{
			ReproduceFemales -> push_back( this );
			GermState = REPRODUCING;
		}
	}
}

void Germ::Crawl( D3DXVECTOR2 objective )
{
	GermState = CRAWLING;
	InitialPosition = Position;
	FinalPosition = objective;
}

void Germ::Rotate( float objective )
{
	GermState = ROTATING;
	InitialRotation = Rotation;
	FinalRotation = objective;
}

class Food
{
	public:
		
		Food( D3DXVECTOR2 pos ) { Position = pos; }
		~Food() {}
		
		D3DXVECTOR2 GetPosition() { return Position; }
		
		D3DXVECTOR2 Position;
};

class Water
{
	public:
		
		Water( D3DXVECTOR2 pos ) { Position = pos; }
		~Water() {}
		
		D3DXVECTOR2 GetPosition() { return Position; }
		
		D3DXVECTOR2 Position;
};

class AIProject
{
	public:
		
		AIProject();
		~AIProject() {}
		
		void InitAI();
		void Update();
		void RenderScene();
		
	private:
		HWND m_hWnd;
		LPDIRECT3D9 m_pD3d;
		LPDIRECT3DDEVICE9 m_pDevice;
		
		float m_fScale;
		
		std::vector< Germ* > m_vGerms;
		std::vector< Food* > m_vFood;
		std::vector< Water* > m_vWater;
		
		std::vector< Germ* > m_vReproduceFemales;
		
		static LRESULT CALLBACK WndProc( HWND, UINT, WPARAM, LPARAM );
};

AIProject::AIProject()
{
	m_fScale = 1.0f;
	
	WNDCLASSEX wc;
	MSG msg;
	
	wc = ( WNDCLASSEX )
	{
		sizeof( WNDCLASSEX ), 0, WndProc, 0, 0, 
		GetModuleHandle( NULL ), LoadIcon( NULL, IDI_APPLICATION ), 
		LoadCursor( NULL, IDC_ARROW ), ( HBRUSH )( COLOR_WINDOW +1 ), 
		NULL, "Class", LoadIcon( NULL, IDI_APPLICATION ),
	};
	RegisterClassEx( &wc );
	
	m_hWnd = CreateWindowEx( 0, wc.lpszClassName, "First AI Project", WS_VISIBLE | WS_OVERLAPPEDWINDOW, 0, 0, 800, 600, NULL, NULL, NULL, 0 );
	RECT rc;
	GetWindowRect( m_hWnd, &rc );
	SetWindowPos( m_hWnd, NULL, GetSystemMetrics( SM_CXSCREEN ) / 2 - rc.right / 2, GetSystemMetrics( SM_CYSCREEN ) / 2 - rc.bottom / 2, 0, 0, SWP_NOSIZE );
	
	m_pD3d = Direct3DCreate9( D3D_SDK_VERSION );
	
	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory( &d3dpp, sizeof( d3dpp ) );
	
	d3dpp.Windowed = true;
	d3dpp.hDeviceWindow = m_hWnd;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
	d3dpp.BackBufferWidth = GetSystemMetrics( SM_CXSCREEN );
	d3dpp.BackBufferHeight = GetSystemMetrics( SM_CYSCREEN );
	d3dpp.EnableAutoDepthStencil = true;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
	
	m_pD3d -> CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, m_hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &m_pDevice );
	
	InitAI();
	
	while( WM_QUIT != msg.message )
	{
		if( PeekMessage( &msg, 0, 0, 0, PM_REMOVE ) )
		{
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
		else
		{
			Update();
			RenderScene();
		}
	}
}

void AIProject::InitAI()
{
	D3DVIEWPORT9 view;
	m_pDevice -> GetViewport( &view );
	
	srand( timeGetTime() );
	for( int i = 0; i < 10; i++ )
	{
		m_vGerms.push_back( new Germ( D3DXVECTOR2( 450, 300 ), rand() % 2, &m_vReproduceFemales ) );
	}
	for( int i = 0; i < 100; i++ )
	{
		m_vFood.push_back( new Food( D3DXVECTOR2( rand() % (int)view.Width, rand() % (int)view.Height ) ) );
	}
	for( int i = 0; i < 200; i++ )
	{
		m_vWater.push_back( new Water( D3DXVECTOR2( rand() % (int)view.Width, rand() % (int)view.Height ) ) );
	}
}

void AIProject::Update()
{
	static int LastTime = timeGetTime();
	int ElapsedTime = timeGetTime() - LastTime;
	float fTime = ( float ) ElapsedTime / 1000.0f;
	LastTime = timeGetTime();
	
	if( GetKeyState( VK_ADD ) < 0 ) m_fScale += 1.0f * fTime;
	if( GetKeyState( VK_SUBTRACT ) < 0 ) m_fScale -= 1.0f * fTime;
	
	
	D3DXMATRIX Rotation;
	srand( timeGetTime() );
	for( int i = 0; i < m_vGerms.size(); i++ )
	{
		switch( m_vGerms[ i ] -> GetState() )
		{
			case Germ::WAITING: {
				if( m_vGerms[ i ] -> GetFemale() == NULL )
				{
					if( rand() % 100 > 20 )
					{
						D3DXMatrixRotationZ( &Rotation, m_vGerms[ i ] -> GetRotation() );
						D3DXVECTOR2 forward( 0, rand() % 100 );
						D3DXVec2TransformCoord( &forward, &forward, &Rotation );
						m_vGerms[ i ] -> Crawl( forward );
					}
					else
					{
						m_vGerms[ i ] -> Rotate( M_PI / 100 * ( rand() % 100 ) * ( rand() % 2 == 0 ? -1 : 1 ) );
					}
				}
				else if( !m_vGerms[ i ] -> GetInPosition() )
				{
					m_vGerms[ i ] -> Crawl( m_vGerms[ i ] -> GetFemale() -> GetPosition() - m_vGerms[ i ] -> GetPosition() );
				}
				else 
				{
					m_vGerms.push_back( new Germ( m_vGerms[ i ] -> GetFemale() -> GetPosition(), rand() % 2, &m_vReproduceFemales ) );
					m_vGerms[ i ] -> GetFemale() -> SetState( Germ::WAITING );
					m_vGerms[ i ] -> GetFemale() -> SetReproduceNeed( 0.0f );
					m_vGerms[ i ] -> SetFemaleNull();
					m_vGerms[ i ] -> SetReproduceNeed( 0.0f );
				}
				m_vGerms[ i ] -> SetInPosition( false );
				break;
			}
			case Germ::CRAWLING: {
				if( Vec2Distance( m_vGerms[ i ] -> GetPosition(), m_vGerms[ i ] -> GetObjective() + m_vGerms[ i ] -> GetInitialPosition() ) > 10.0f )
				{
					float Dist = Vec2Distance( D3DXVECTOR2( 0, 0 ), m_vGerms[ i ] -> GetObjective() );
					D3DXVECTOR2 sincos = D3DXVECTOR2( m_vGerms[ i ] -> GetObjective().x / Dist, m_vGerms[ i ] -> GetObjective().y / Dist );
					D3DXVECTOR2 forward( Germ::speed * fTime * sincos.x, Germ::speed * fTime * sincos.y );
					m_vGerms[ i ] -> SetPosition( m_vGerms[ i ] -> GetPosition() += forward );
				}
				else
				{
					m_vGerms[ i ] -> SetState( Germ::WAITING );
					m_vGerms[ i ] -> SetInPosition( true );
				}
				break;
			}
			case Germ::ROTATING: {
				if( fabs( m_vGerms[ i ] -> GetRotationObjective() - m_vGerms[ i ] -> GetRotation() ) > M_PI / 45 )
				{
					m_vGerms[ i ] -> SetRotation( m_vGerms[ i ] -> GetRotation() + Germ::rspeed * fTime * SIGNAL( m_vGerms[ i ] -> GetRotationObjective() ) );
					while( fabs( m_vGerms[ i ] -> GetRotation() ) >= M_PI )
					{
						m_vGerms[ i ] -> SetRotation( m_vGerms[ i ] -> GetRotation() - M_PI * 2 * SIGNAL( m_vGerms[ i ] -> GetRotation() ) );
					}
				}
				else
				{
					m_vGerms[ i ] -> SetState( Germ::WAITING );
				}
				break;
			}
		}
		
		D3DVIEWPORT9 view;
		m_pDevice -> GetViewport( &view );
		
		D3DXVECTOR4 box( 0, 0, view.Width, view.Height );
		m_vGerms[ i ] -> SetPosition( AdjustVecInBox( m_vGerms[ i ] -> GetPosition(), box, true ) );
		m_vGerms[ i ] -> Mind( fTime );
	}
}

void AIProject::RenderScene()
{
	m_pDevice -> Clear( 0, NULL, D3DCLEAR_ZBUFFER | D3DCLEAR_TARGET, D3DCOLOR_XRGB( 255, 255, 255 ), 1.0, 0);
	m_pDevice -> BeginScene();
	
	D3DXMATRIX Zoom;
	D3DXMatrixScaling( &Zoom, m_fScale, m_fScale, 1.0f );
	
	static const D3DXVERTEX GermVert[ 4 ] = 
	{
		{ D3DXVECTOR4( -1.0f, 0.0f, 0.5f, 1.0f ), D3DCOLOR_XRGB( 0, 0, 0 ) }, 
		{ D3DXVECTOR4( 1.0f, 0.0f, 0.5f, 1.0f ), D3DCOLOR_XRGB( 0, 0, 0 ) }, 
		{ D3DXVECTOR4( 0.0f, 0.0f, 0.5f, 1.0f ), D3DCOLOR_XRGB( 0, 0, 0 ) }, 
		{ D3DXVECTOR4( 0.0f, 2.0f, 0.5f, 1.0f ), D3DCOLOR_XRGB( 0, 0, 0 ) },
	};
	
	D3DXVERTEX VertTemp[ 4 ];
	
	VertTemp[ 0 ].color = GermVert[ 0 ].color;
	VertTemp[ 1 ].color = GermVert[ 1 ].color;
	VertTemp[ 2 ].color = GermVert[ 2 ].color;
	VertTemp[ 3 ].color = GermVert[ 3 ].color;
	
	D3DXMATRIX Transform;
	D3DXMATRIX Translate;
	D3DXMATRIX Rotation;
	D3DXMATRIX Scale;
	
	D3DXMatrixScaling( &Scale, 5.0f, 5.0f, 1.0f );
	
	for( int i = 0; i < m_vGerms.size(); i++ )
	{
		D3DXMatrixTranslation( &Translate, m_vGerms[ i ] -> GetPosition().x, m_vGerms[ i ] -> GetPosition().y, 0.0f );
		D3DXMatrixRotationZ( &Rotation, m_vGerms[ i ] -> GetRotation() );
		Transform = Scale * Rotation * Translate * Zoom;
		
		if( m_vGerms[ i ] -> GetSex() )
		{
			VertTemp[ 2 ].color = D3DCOLOR_XRGB( 255, 0, 0 );
			VertTemp[ 3 ].color = D3DCOLOR_XRGB( 255, 0, 0 );
		}
		else
		{
			VertTemp[ 2 ].color = D3DCOLOR_XRGB( 0, 0, 0 );
			VertTemp[ 3 ].color = D3DCOLOR_XRGB( 0, 0, 0 );
		}
		
		D3DXVec4Transform( &VertTemp[ 0 ].pos, &GermVert[ 0 ].pos, &Transform );
		D3DXVec4Transform( &VertTemp[ 1 ].pos, &GermVert[ 1 ].pos, &Transform );
		D3DXVec4Transform( &VertTemp[ 2 ].pos, &GermVert[ 2 ].pos, &Transform );
		D3DXVec4Transform( &VertTemp[ 3 ].pos, &GermVert[ 3 ].pos, &Transform );
		
		m_pDevice -> SetFVF( D3DXVERTEX::FVF );
		m_pDevice -> DrawPrimitiveUP( D3DPT_LINELIST, 2, VertTemp, sizeof( D3DXVERTEX ) );
	}
	
	static const D3DXVERTEX FoodVert[ 4 ] = 
	{
		{ D3DXVECTOR4( 1.0f, 1.0f, 0.5f, 1.0f ), D3DCOLOR_XRGB( 165, 42, 42 ) }, 
		{ D3DXVECTOR4( -1.0f, 1.0f, 0.5f, 1.0f ), D3DCOLOR_XRGB( 165, 42, 42 ) }, 
		{ D3DXVECTOR4( 1.0f, -1.0f, 0.5f, 1.0f ), D3DCOLOR_XRGB( 165, 42, 42 ) }, 
		{ D3DXVECTOR4( -1.0f, -1.0f, 0.5f, 1.0f ), D3DCOLOR_XRGB( 165, 42, 42 ) },
	};
	
	VertTemp[ 0 ].color = FoodVert[ 0 ].color;
	VertTemp[ 1 ].color = FoodVert[ 1 ].color;
	VertTemp[ 2 ].color = FoodVert[ 2 ].color;
	VertTemp[ 3 ].color = FoodVert[ 3 ].color;
	
	D3DXMatrixScaling( &Scale, 3.0f, 3.0f, 1.0f );
	
	for( int i = 0; i < m_vFood.size(); i++ )
	{
		D3DXMatrixTranslation( &Translate, m_vFood[ i ] -> GetPosition().x, m_vFood[ i ] -> GetPosition().y, 0.0f );
		Transform = Scale * Translate * Zoom;
		
		D3DXVec4Transform( &VertTemp[ 0 ].pos, &FoodVert[ 0 ].pos, &Transform );
		D3DXVec4Transform( &VertTemp[ 1 ].pos, &FoodVert[ 1 ].pos, &Transform );
		D3DXVec4Transform( &VertTemp[ 2 ].pos, &FoodVert[ 2 ].pos, &Transform );
		D3DXVec4Transform( &VertTemp[ 3 ].pos, &FoodVert[ 3 ].pos, &Transform );
		
		m_pDevice -> SetFVF( D3DXVERTEX::FVF );
		m_pDevice -> DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 2, VertTemp, sizeof( D3DXVERTEX ) );
	}
	
	static const D3DXVERTEX WaterVert[ 3 ] =
	{
		{ D3DXVECTOR4( 0.0f, 1.0f, 0.5f, 1.0f ), D3DCOLOR_XRGB( 127, 127, 255 ) }, 
		{ D3DXVECTOR4( -1.0f, 0.0f, 0.5f, 1.0f ), D3DCOLOR_XRGB( 127, 127, 255 ) }, 
		{ D3DXVECTOR4( 1.0f, 0.0f, 0.5f, 1.0f ), D3DCOLOR_XRGB( 127, 127, 255 ) }, 
	};
	
	D3DXVERTEX VertTemp2[ 3 ];
	VertTemp2[ 0 ].color = WaterVert[ 0 ].color;
	VertTemp2[ 1 ].color = WaterVert[ 1 ].color;
	VertTemp2[ 2 ].color = WaterVert[ 2 ].color;
	
	D3DXMatrixScaling( &Scale, 5.0f, 5.0f, 1.0f );
	
	for( int i = 0; i < m_vWater.size(); i++ )
	{
		D3DXMatrixTranslation( &Translate, m_vWater[ i ] -> GetPosition().x, m_vWater[ i ] -> GetPosition().y, 0.0f );
		Transform = Scale * Translate * Zoom;
		
		D3DXVec4Transform( &VertTemp2[ 0 ].pos, &WaterVert[ 0 ].pos, &Transform );
		D3DXVec4Transform( &VertTemp2[ 1 ].pos, &WaterVert[ 1 ].pos, &Transform );
		D3DXVec4Transform( &VertTemp2[ 2 ].pos, &WaterVert[ 2 ].pos, &Transform );
		
		m_pDevice -> SetFVF( D3DXVERTEX::FVF );
		m_pDevice -> DrawPrimitiveUP( D3DPT_TRIANGLELIST, 1, VertTemp2, sizeof( D3DXVERTEX ) );
	}
	
	m_pDevice -> EndScene();
	m_pDevice -> Present( NULL, NULL, NULL, NULL );
}

float Vec2Distance( D3DXVECTOR2 vec1, D3DXVECTOR2 vec2 )
{
	vec1 = vec2 - vec1;
	return sqrt( vec1.x * vec1.x + vec1.y * vec1.y );
}

D3DXVECTOR2 AdjustVecInBox( D3DXVECTOR2 vec, D3DXVECTOR4 box, bool pass )
{
	if( !pass )
	{
		if( vec.x < box.x )
			vec.x = box.x;
		if( vec.y < box.y )
			vec.y = box.y;
		if( vec.x > box.z )
			vec.x = box.z;
		if( vec.y > box.w )
			vec.y = box.w;
	}
	else
	{
		if( vec.x < box.x )
			vec.x = box.z;
		if( vec.y < box.y )
			vec.y = box.w;
		if( vec.x > box.z )
			vec.x = box.x;
		if( vec.y > box.w )
			vec.y = box.y;
	}
	
	return vec;
}

LRESULT CALLBACK AIProject::WndProc( HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam )
{
	switch( Message )
	{
		case WM_DESTROY: {
			PostQuitMessage( 0 );
			break;
		}
		default:
			return DefWindowProc( hWnd, Message, wParam, lParam );
	}
}

int main()
{
	AIProject aip;
	aip = AIProject();
}
