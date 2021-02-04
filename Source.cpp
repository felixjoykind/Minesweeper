#define OLC_PGE_APPLICATION

#include "olcPixelGameEngine.h"
#include <iostream>
#include <algorithm>

class Game : public olc::PixelGameEngine
{
public:
	Game()
	{
		sAppName = "Minesweeper";
	}

private:
	enum class TileType
	{
		Hidden,
		Flagged,
		Revealed
	};

	class Tile
	{
	public:
		Tile(bool mined, TileType tileType = TileType::Hidden)
			: type(tileType), hasBomb(mined) {};
		TileType GetType() const { return type; }
		void SetType(TileType newType) { type = newType; }

		bool Mined() const { return hasBomb; }
		void SpawnBomb() { hasBomb = true; }
	private:
		TileType type;
		bool hasBomb;
	};

	// sprites
	olc::Sprite* hiddenTileSpr = new olc::Sprite("./TilesSprites/tile_hidden.png");
	olc::Sprite* revealedTileSpr = new olc::Sprite("./TilesSprites/tile_revealed.png");
	olc::Sprite* minedTileSpr = new olc::Sprite("./TilesSprites/tile_mined.png");
	olc::Sprite* flaggedTileSpr = new olc::Sprite("./TilesSprites/tile_flagged.png");
	olc::Sprite* wrongFlaggedTileSpr = new olc::Sprite("./TilesSprites/tile_flagged_wrong.png");

	std::vector<Tile> tiles;
	int nWidth = 16;
	int nHeight = 16;

	int nTile = 16;

	bool losed = false;
	bool won = false;

	const unsigned int bombsLimit = 30;
	unsigned int bombCount = 0;

	const unsigned int flagsLimit = 32;
	int unsigned flagsCount = 0;

	const int offsetY = 30;

	int CountNeighbours(const int x, const int y)
	{
		const int xStart = std::max(0, x-1);
		const int yStart = std::max(0, y-1);
		const int xEnd = std::min(nWidth - 1, x+1);
		const int yEnd = std::min(nHeight - 1, y+1);

		int count = 0;
		for (int y = yStart; y <= yEnd; y++)
		{
			for (int x = xStart; x <= xEnd; x++)
			{
				if (tiles[y * nWidth + x].Mined())
					count++;
			}
		}
		return count;
	}

	bool OnUserCreate() override
	{
		for (int i = 0; i < 256; i++)
			tiles.push_back(Tile(false));

		srand(time(NULL));
		for (int i = 0; i <= bombsLimit; i++)
		{
			olc::vi2d pos;
			do
			{
				pos.x = rand() % 16;
				pos.y = rand() % 16;
			} while (tiles[pos.y * nWidth + pos.x].Mined());
			tiles[pos.y * nWidth + pos.x].SpawnBomb();
			std::cout << "bomb spawned\n";
		}

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		// lambda functions
		auto GetTile = [&](int x, int y)
		{
			if ((x >= 0 && x < nWidth) && (y >= 0 && y < nHeight))
				return tiles[y * nWidth + x];
		};

		auto SetTile = [&](int x, int y, TileType newType)
		{
			if ((x >= 0 && x < nWidth) && (y >= 0 && y < nHeight))
				tiles[y * nWidth + x].SetType(newType);
		};

		// getting mouse input
		olc::vf2d mouse = { float(GetMouseX()), float(GetMouseY())};
		if (!losed)
		{
			int x = mouse.x / nTile;
			int y = (mouse.y - offsetY) / nTile;
			auto tile = GetTile(x, y); // pressed tile
			if (GetMouse(0).bPressed)
			{
				if (tile.GetType() == TileType::Hidden)
				{
					SetTile(x, y, TileType::Revealed);
					std::cout << "Revealed tile: " << x << ", " << y << std::endl;
					if (tile.Mined())
					{
						losed = true;
						std::cout << "Losed :(" << std::endl;
						// revealing all mines if player losed
						for (auto& tile : tiles)
						{
							if (tile.Mined() && tile.GetType() == TileType::Hidden)
								tile.SetType(TileType::Revealed);
						}
					}
				}
			}
			if (GetMouse(1).bPressed)
			{
				// flag if tile is not already flagged and revealed
				if (tile.GetType() != TileType::Flagged && tile.GetType() != TileType::Revealed && flagsCount <= flagsLimit)
				{
					SetTile(x, y, TileType::Flagged);
					std::cout << "Flagged tile: " << x << ", " << y << std::endl;
					flagsCount++;
					won = std::count_if(tiles.begin(), tiles.end(), [](Tile t) { return t.Mined() && t.GetType() == TileType::Flagged; }) == bombCount
						&& flagsCount == bombCount;
					// revealing all tiles if player won
					if (won)
					{
						for (auto& tile : tiles)
						{
							if (tile.GetType() == TileType::Hidden)
								tile.SetType(TileType::Revealed);
						}
					}
				}
				// unflag if tile is flagged
				else if (tile.GetType() == TileType::Flagged)
				{
					SetTile(x, y, TileType::Hidden);
					std::cout << "Unflagged tile: " << x << ", " << y << std::endl;
					flagsCount--;
				}
			}
		}
		else if (losed || won)
		{
			if (GetKey(olc::Key::SPACE).bPressed)
			{
				tiles.clear();
				bombCount = 0;
				flagsCount = 0;

				for (int i = 0; i < 256; i++)
					tiles.push_back(Tile(false));

				srand(time(NULL));
				for (int i = 0; i <= bombsLimit; i++)
				{
					olc::vi2d pos;
					do
					{
						pos.x = rand() % 16;
						pos.y = rand() % 16;
					} while (tiles[pos.y * nWidth + pos.x].Mined());
					tiles[pos.y * nWidth + pos.x].SpawnBomb();
					std::cout << "bomb spawned\n";
				}
				losed = false;
				won = false;
			}
		}

		Clear(olc::DARK_GREY);
		// drawing tiles
		for (int y = 0; y < nHeight; y++)
		{
			for (int x = 0; x < nWidth; x++)
			{
				auto tile = GetTile(x, y);
				int neightbours = CountNeighbours(x, y);
				switch (tile.GetType())
				{
				case TileType::Hidden:
					DrawSprite({ x * nTile, y * nTile + offsetY }, hiddenTileSpr, 2U);
					break;
				case TileType::Revealed:
					if (tile.Mined())
						DrawSprite({ x* nTile, y * nTile + offsetY }, minedTileSpr, 2U);
					else
					{
						DrawSprite({ x * nTile, y * nTile + offsetY }, revealedTileSpr, 2U);
						if (neightbours > 0)
							DrawString({ x * nTile + 4, y * nTile + 4 + offsetY }, std::to_string(neightbours), olc::DARK_BLUE);
					}
					break;
				case TileType::Flagged:
					DrawSprite({ x * nTile, y * nTile + offsetY }, losed && !tile.Mined() ? wrongFlaggedTileSpr : flaggedTileSpr, 2U);
					break;
				default:
					break;
				}
			}
		}

		if (losed)
		{
			DrawString(ScreenWidth() / 2 - 10, 8, ":(", olc::RED, 2U);
			DrawString(ScreenWidth() - 90, 8, "Press SPACE\nto restart", olc::BLACK, 1U);
		}
		else if (won)
		{
			DrawString(ScreenWidth() / 2 - 10, 8, "xD", olc::RED, 2U);
			DrawString(ScreenWidth() - 90, 8, "Press SPACE\nto restart", olc::BLACK, 1U);
		}
		else
			DrawString(ScreenWidth() / 2 - 10, 8, ":)", olc::RED, 2U);
		// displaying game stats
		DrawString(5, 8, std::to_string(flagsLimit - flagsCount), olc::RED, 2U);

		return true;
	}
};

int main()
{
	Game game;
	if (game.Construct(256, 286, 2, 2))
		game.Start();
	return 0;
}