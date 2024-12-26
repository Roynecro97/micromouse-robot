#ifndef UNITTESTS_MAZE_SOLVER_ADAPTERS_H
#define UNITTESTS_MAZE_SOLVER_ADAPTERS_H

#include <maze_solver/cell.h>

#include <ostream>
#include <utility>

namespace micromouse
{

inline std::ostream &operator<<(std::ostream &os, Walls walls)
{
    using rep_type = std::underlying_type_t<Walls>;
    static constexpr rep_type reminder_mask = 0xF0;
    static_assert(reminder_mask == rep_type(~std::to_underlying(full_walls)));
    rep_type reminder = std::to_underlying(walls) & reminder_mask;
    os << enum2str(walls);
    if (reminder != 0)
    {
        const auto flags = os.flags(os.hex);
        os << "|0x" << +reminder;
        os.flags(flags);
    }
    return os;
}

namespace detail
{
constexpr char bit2char(bool bit) noexcept
{
    return bit ? '1' : '0';
}
}  // namespace detail

inline std::ostream &operator<<(std::ostream &os, Cell cell)
{
    const auto flags = os.flags(os.boolalpha);
    os << "Cell{" << cell.walls << "," << (cell.visited ? "" : " not") << " visited, f = " << detail::bit2char(cell.f1)
       << detail::bit2char(cell.f2) << detail::bit2char(cell.f3) << "}";
    os.flags(flags);
    return os;
}

}  // namespace micromouse

#endif  // UNITTESTS_MAZE_SOLVER_ADAPTERS_H
