/****************************************************************************************** 
 *	Chili DirectX Framework Version 14.03.22											  *	
 *	Game.cpp																			  *
 *	Copyright 2014 PlanetChili.net <http://www.planetchili.net>							  *
 *																						  *
 *	This file is part of The Chili DirectX Framework.									  *
 *																						  *
 *	The Chili DirectX Framework is free software: you can redistribute it and/or modify	  *
 *	it under the terms of the GNU General Public License as published by				  *
 *	the Free Software Foundation, either version 3 of the License, or					  *
 *	(at your option) any later version.													  *
 *																						  *
 *	The Chili DirectX Framework is distributed in the hope that it will be useful,		  *
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of						  *
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the						  *
 *	GNU General Public License for more details.										  *
 *																						  *
 *	You should have received a copy of the GNU General Public License					  *
 *	along with The Chili DirectX Framework.  If not, see <http://www.gnu.org/licenses/>.  *
 ******************************************************************************************/
#include "Game.h"
#include "DirectXTK\Inc\SpriteBatch.h"
#include <DirectXMath.h>
#include "DirectXTK\Inc\CommonStates.h"
#include <iomanip>
#include <sstream>
#include <string>

Game::Game( HWND hWnd,KeyboardServer& kServer,MouseServer& mServer )
	:
	hWnd( hWnd ),
	kbd( kServer ),
	mouse( mServer ),
	gfx( hWnd ),
	twinkle( L"twinkle.png",RECT { 0,0,8,8 },1.0f,{ 4.0f,4.0f },gfx.GetTexMan() ),
	sim( 2000 ),
	logFile( L"logfile.txt",std::ios_base::out )
{
}

Game::~Game()
{
}

void Game::Go()
{
	gfx.BeginFrame();	
	UpdateModel();
	ComposeFrame();
	gfx.EndFrame();
}

void Game::UpdateModel()
{
	ft.StartFrame();
	sim.Step();
	if( !ft.StopFrame() )
	{
		return;
	}
	else
	{
		std::wstringstream ss;
		ss.precision( 3 );
		ss << L"Avg: [" << std::fixed << ft.GetAvg() << L"] Min: [" << ft.GetMin()
			<< L"] Max: [" << ft.GetMax() << L"]" << std::endl;
		logFile << ss.str();
	}
}

void Game::ComposeFrame()
{
	auto batch = gfx.MakeSpriteBatch();
	auto pointSampling = gfx.MakeSamplerState();
	auto states = gfx.MakeStates();

	batch->Begin( DirectX::SpriteSortMode::SpriteSortMode_Deferred,
		states->NonPremultiplied(),
		pointSampling.Get() );
	sim.Draw( twinkle,*batch );
	batch->End();
}
