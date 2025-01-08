#ifndef MAZE_SOLVER_MAZE_H
#define MAZE_SOLVER_MAZE_H

#include <misc_utils/typing_utils.h>

#include "cell.h"

#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <cstring>
#include <type_traits>
#include <utility>

namespace micromouse
{

namespace detail
{
template <typename T>
concept AnyCell = std::is_same_v<T, Walls> || std::is_same_v<T, Cell>;
}  // namespace detail

template <std::size_t Rows = 8 /* 16 */, std::size_t Columns = Rows>
class Maze
{
    static constexpr auto protect_bounding_walls(std::size_t row, std::size_t col, Walls walls) noexcept
    {
        if (row == 0)
        {
            walls |= Walls::North;
        }
        else if (row == Rows - 1)
        {
            walls |= Walls::South;
        }
        if (col == 0)
        {
            walls |= Walls::West;
        }
        else if (col == Columns - 1)
        {
            walls |= Walls::East;
        }
        return walls;
    }

    class CellWallsProxy
    {
    public:
        constexpr CellWallsProxy(Maze &m, std::size_t row, std::size_t col) noexcept : m_maze(m), m_row(row), m_col(col)
        {
        }

        constexpr operator Walls() const noexcept { return exact().walls; }
        constexpr CellWallsProxy &operator=(Walls val) && noexcept
        {
            exact().walls = protect_bounding_walls(m_row, m_col, val);
            m_maze.add_neighbor_walls(m_row, m_col, val);
            m_maze.remove_neighbor_walls(m_row, m_col, ~val);
            return *this;
        }
        constexpr CellWallsProxy &operator|=(Walls val) && noexcept
        {
            m_maze.add_walls(m_row, m_col, val);
            return *this;
        }
        constexpr CellWallsProxy &operator&=(Walls val) && noexcept
        {
            m_maze.remove_walls(m_row, m_col, ~val);
            return *this;
        }
        constexpr CellWallsProxy &operator^=(Walls val) && noexcept
        {
            exact() ^= val;
            m_maze.add_neighbor_walls(m_row, m_col, exact().walls);
            m_maze.remove_neighbor_walls(m_row, m_col, ~exact().walls);
            return *this;
        }

        friend constexpr Walls operator|(const CellWallsProxy &lhs, const CellWallsProxy &rhs) noexcept
        {
            return lhs.exact().walls | rhs.exact().walls;
        }
        friend constexpr Walls operator&(const CellWallsProxy &lhs, const CellWallsProxy &rhs) noexcept
        {
            return lhs.exact().walls & rhs.exact().walls;
        }
        friend constexpr Walls operator^(const CellWallsProxy &lhs, const CellWallsProxy &rhs) noexcept
        {
            return lhs.exact().walls ^ rhs.exact().walls;
        }
        friend constexpr Walls operator~(const CellWallsProxy &proxy) noexcept { return ~proxy.exact().walls; }

        friend constexpr Walls operator|(const CellWallsProxy &lhs, Walls rhs) noexcept
        {
            return lhs.exact().walls | rhs;
        }
        friend constexpr Walls operator&(const CellWallsProxy &lhs, Walls rhs) noexcept
        {
            return lhs.exact().walls & rhs;
        }
        friend constexpr Walls operator^(const CellWallsProxy &lhs, Walls rhs) noexcept
        {
            return lhs.exact().walls ^ rhs;
        }
        friend constexpr Walls operator|(Walls lhs, const CellWallsProxy &rhs) noexcept
        {
            return lhs | rhs.exact().walls;
        }
        friend constexpr Walls operator&(Walls lhs, const CellWallsProxy &rhs) noexcept
        {
            return lhs & rhs.exact().walls;
        }
        friend constexpr Walls operator^(Walls lhs, const CellWallsProxy &rhs) noexcept
        {
            return lhs ^ rhs.exact().walls;
        }

    private:
        constexpr Cell &exact() noexcept { return m_maze.cells[m_row][m_col]; }
        constexpr const Cell &exact() const noexcept { return m_maze.cells[m_row][m_col]; }

        Maze &m_maze;
        std::size_t m_row;
        std::size_t m_col;
    };

public:
    using size_type = std::size_t;
    using value_type = Cell;

    constexpr Maze() noexcept : cells{}
    {
        for (auto col = 0UZ; col < Columns; col++)
        {
            cells[0][col] |= Walls::North;
            cells[Rows - 1][col] |= Walls::South;
        }

        for (auto row = 0UZ; row < Rows; row++)
        {
            cells[row][0] |= Walls::West;
            cells[row][Columns - 1] |= Walls::East;
        }
    }
    explicit constexpr Maze(const detail::AnyCell auto (&raw_walls)[Rows][Columns]) noexcept
        : cells(std::bit_cast<decltype(cells)>(raw_walls))
    {
    }
    template <detail::AnyCell C>
    explicit constexpr Maze(const std::array<std::array<C, Columns>, Rows> &raw_walls) noexcept
        : cells(std::bit_cast<decltype(cells)>(raw_walls))
    {
    }

    static constexpr size_type size() { return Rows * Columns; }
    static constexpr size_type height() { return Rows; }
    static constexpr size_type width() { return Columns; }

    constexpr auto operator[](size_type row, size_type col) & noexcept { return CellWallsProxy(*this, row, col); }
    constexpr auto operator[](size_type row, size_type col) && noexcept { return cells[row][col].walls; }
    constexpr auto operator[](size_type row, size_type col) const & noexcept { return cells[row][col].walls; }
    constexpr auto operator[](size_type row, size_type col) const && noexcept { return cells[row][col].walls; }

    constexpr void add_walls(size_type row, size_type col, Walls walls) noexcept
    {
        cells[row][col] |= walls;
        add_neighbor_walls(row, col, walls);
    }

    constexpr void remove_walls(size_type row, size_type col, Walls walls) noexcept
    {
        cells[row][col] &= protect_bounding_walls(row, col, ~walls);
        remove_neighbor_walls(row, col, walls);
    }

private:
    /**
     * @brief Add walls to the neighboring cells in the maze.
     *
     * @param row The row of the cell to add walls around.
     * @param col The column of the cell to add walls around.
     * @param walls The walls to add.
     */
    constexpr void add_neighbor_walls(size_type row, size_type col, Walls walls) noexcept
    {
        if ((Walls::North & walls) != empty_walls && row > 0)
        {
            cells[row - 1][col] |= Walls::South;
        }
        if ((Walls::East & walls) != empty_walls && col < Columns - 1)
        {
            cells[row][col + 1] |= Walls::West;
        }
        if ((Walls::South & walls) != empty_walls && row < Rows - 1)
        {
            cells[row + 1][col] |= Walls::North;
        }
        if ((Walls::West & walls) != empty_walls && col > 0)
        {
            cells[row][col - 1] |= Walls::East;
        }
    }

    /**
     * @brief Remove walls from the neighboring cells in the maze.
     * @note Unlike the Python version, this method does nothing when attempting to remove bounding walls instead of
     *       failing.
     *
     * @param row The row of the cell to remove walls around.
     * @param col The column of the cell to remove walls around.
     * @param walls The walls to remove.
     */
    constexpr void remove_neighbor_walls(size_type row, size_type col, Walls walls) noexcept
    {
        if ((Walls::North & walls) != empty_walls && row > 0)
        {
            cells[row - 1][col] &= ~Walls::South;
        }
        if ((Walls::East & walls) != empty_walls && col < Columns - 1)
        {
            cells[row][col + 1] &= ~Walls::West;
        }
        if ((Walls::South & walls) != empty_walls && row < Rows - 1)
        {
            cells[row + 1][col] &= ~Walls::North;
        }
        if ((Walls::West & walls) != empty_walls && col > 0)
        {
            cells[row][col - 1] &= ~Walls::East;
        }
    }

    std::array<std::array<value_type, Columns>, Rows> cells;
};

extern template class Maze<>;

}  // namespace micromouse

#endif  // MAZE_SOLVER_MAZE_H
