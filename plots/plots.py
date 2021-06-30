import time
import ijson
import numpy as np
import matplotlib
import matplotlib.pyplot as plt
import matplotlib.patheffects as pe
from cycler import cycler

# apply custom style sheet globally
plt.style.use("custom-dark")


def tick_to_time(tick):
    return time.gmtime(round(tick / 15.63))


def get_tick(filename):
    with open(filename, "rb") as f:
        object = ijson.items(f, "stats.tick")
        for data in object:
            return data


# get json data of player i by property ["name", "team", "race", "score", "settlers",
# "buildings", "food", "mines", "gold", "manna", "soldiers", "battles"]
def get_player_values(filename, player_number, property):
    with open(filename, "rb") as f:
        object = ijson.items(f, "stats.player" + str(player_number) + "." + property)
        for data in object:
            return data


# get json about game by property ["date", "time", "numPlayers", "gameEnded"]
def get_general_values(filename, property):
    with open(filename, "rb") as f:
        object = ijson.items(f, "general." + property)
        for data in object:
            return data


# get array with all player names and array with slot number
def get_all_names(filename):
    number_of_players = get_general_values(filename, "numPlayers")
    player_names = []
    slots = []
    count = 0
    for i in range(20):
        player_name = get_player_values(filename, player_number=i, property="name")
        if player_name != "":
            player_names.append(player_name)
            slots.append(i)
            count += 1
            if count is number_of_players:
                break

    return player_names, slots


# append multiple saves
def combine_ticks(filename_list):
    combined_values = np.array([], dtype=np.int64)
    for filename in filename_list:
        values = np.array(get_tick(filename), dtype=np.int64)
        combined_values = np.append(combined_values, values, axis=0)

    return combined_values


def combine_values(filename_list, player_number, property):
    combined_values = np.array([], dtype=np.int64)
    for filename in filename_list:
        values = np.array(
            get_player_values(filename, player_number, property), dtype=np.int64
        )
        combined_values = np.append(combined_values, values, axis=0)

    return combined_values


# stack all players to 2d-array
def stack_values(filename_list, property):
    stacked_values = combine_values(filename_list, 0, property)
    player_names, slots = get_all_names(filename_list[0])
    for i in slots[1:]:
        combined_values = combine_values(filename_list, i, property)
        stacked_values = np.vstack((stacked_values, combined_values))

    return stacked_values


def plot_stats(filename_list, property, n_xticks=None):
    data = stack_values(filename_list, property)
    x = combine_ticks(filename_list)
    player_names, slots = get_all_names(filename_list[0])
    colors = plt.rcParams["axes.prop_cycle"].by_key()["color"]
    colors = [colors[i] for i in slots]

    # plot curves
    fig, ax = plt.subplots()
    for i in range(data.shape[0]):
        y = data[i][:]
        ax.plot(x, y, label=player_names[i], color=colors[i])

    # formatting
    formatter = matplotlib.ticker.FuncFormatter(
        lambda datapoint, x: time.strftime("%H:%M", tick_to_time(datapoint))
    )
    ax.xaxis.set_major_formatter(formatter)
    if n_xticks:
        ax.xaxis.set_major_locator(plt.MaxNLocator(n_xticks))
    ax.legend()
    ax.set_title(property)
    ax.set_ylim(bottom=0)
    ax.set_xlim(left=0)
    plt.show()


def stack_chart(filename_list):
    soldiers = stack_values(filename_list, property="soldiers") + 21
    # TODO: start soldiers depending on goods
    soldiers_sum = np.sum(soldiers, axis=0)
    soldiers_rel = np.flip(soldiers / soldiers_sum, axis=0)
    x = combine_ticks(filename_list)
    number_of_players = soldiers.shape[0]
    player_names, slots = get_all_names(filename_list[0])

    colors = plt.rcParams["axes.prop_cycle"].by_key()["color"]
    colors = [colors[i] for i in slots]

    # plot stack chart
    fig, ax = plt.subplots()
    plt.stackplot(
        x,
        soldiers_rel,
        labels=player_names,
        colors=colors[:number_of_players][::-1],
        alpha=0.75,
    )
    for i in range(1, number_of_players):  # white lines between stacks
        y = np.sum(soldiers_rel[:i][:], axis=0)
        plt.plot(x, y, linewidth=0.8, color="white", alpha=0.7)

    # formatting
    ax.set_title("Players Soldier Percentage")
    ax.set_ylim(bottom=0, top=1)
    ax.set_xlim(left=0, right=x[-1])
    formatter = matplotlib.ticker.FuncFormatter(
        lambda datapoint, x: time.strftime("%H:%M", tick_to_time(datapoint))
    )
    ax.xaxis.set_major_formatter(formatter)
    ax.yaxis.set_major_locator(plt.MaxNLocator(4))
    ax.yaxis.set_ticklabels([])
    # ax.set_position([0.1, 0.1, 0.7, 0.8])  # TODO enough space for legend

    # legend
    leg = ax.legend(loc="center right", bbox_to_anchor=(-0.01, 0.5))
    for i in range(number_of_players):
        leg.legendHandles[i].set_color(colors[i])
    ax.add_artist(leg)

    # vertical manna lines
    manna = np.flip(stack_values(filename_list, property="manna"), axis=0)
    for i in range(number_of_players):
        manna_l2 = np.where(manna[i][:] >= 10)
        if np.any(manna_l2):
            l2_index = manna_l2[0][:][0]
            ymin = 0
            for j in range(i):
                ymin += soldiers_rel[j][l2_index]
            ymax = ymin + soldiers_rel[i][l2_index]
            plt.vlines(x[l2_index], ymin=ymin, ymax=ymax, linewidth=1.4, colors="grey")
            plt.vlines(x[l2_index], ymin=ymin, ymax=ymax, linewidth=0.8, colors="white")

        manna_l3 = np.where(manna[i][:] >= 110)
        if np.any(manna_l3):
            l3_index = manna_l3[0][:][0]
            ymin = 0
            for j in range(i):
                ymin += soldiers_rel[j][l3_index]
            ymax = ymin + soldiers_rel[i][l3_index]
            plt.vlines(x[l3_index], ymin=ymin, ymax=ymax, linewidth=1.4, colors="grey")
            plt.vlines(x[l3_index], ymin=ymin, ymax=ymax, linewidth=0.8, colors="white")

    plt.show()


def team_chart(filename_list):
    bars = ["Gems", "Gold", "Battles", "Soldiers"]
    soldiers = stack_values(filename_list, property="soldiers")
    battles = stack_values(filename_list, property="battles")
    gold = stack_values(filename_list, property="gold")
    soldiers_0 = 0
    soldiers_1 = 0
    battles_0 = 0
    battles_1 = 0
    gold_0 = 0
    gold_1 = 0
    gems_0 = 0
    gems_1 = 0
    for i in range(soldiers.shape[0]):
        team = get_player_values(filename_list[0], player_number=i, property="team")
        race = get_player_values(filename_list[0], player_number=i, property="race")
        if team == 0:
            soldiers_0 += soldiers[i][-1]
            battles_0 += battles[i][-1]
            if race == 1:
                gems_0 += gold[i][-1]
            else:
                gold_0 += gold[i][-1]
        if team == 1:
            soldiers_1 += soldiers[i][-1]
            battles_1 += battles[i][-1]
            if race == 1:
                gems_1 += gold[i][-1]
            else:
                gold_1 += gold[i][-1]

    x_0 = np.array([gems_0, gold_0, battles_0, soldiers_0])
    x_1 = np.array([gems_1, gold_1, battles_1, soldiers_1])
    y = np.arange(x_0.size)
    maximum = np.max(np.concatenate((x_0, x_1), axis=None))

    # plot curves
    color = plt.rcParams["axes.prop_cycle"].by_key()["color"][1]
    fig, ax = plt.subplots(ncols=2, sharey=True)
    ax[0].barh(y, x_0, align="center", zorder=10)
    ax[1].barh(y, x_1, align="center", color=color, zorder=10)

    # formatting
    ax[0].set(title="Team 1")
    ax[0].set(yticks=y, yticklabels=bars)
    ax[0].yaxis.tick_left()
    ax[0].axis(xmin=maximum * 1.05, xmax=0)
    ax[1].set(title="Team 2")
    ax[1].tick_params(axis="y", which="both", left=False)
    ax[1].axis(xmin=0, xmax=maximum * 1.05)
    # fig.tight_layout()
    fig.subplots_adjust(wspace=0)

    plt.show()


if __name__ == "__main__":
    # testrecording
    filename_list = ["example/2021-05-09_23-19-56.json"]

    plot_stats(filename_list, property="buildings")
    plot_stats(filename_list, property="soldiers")

    stack_chart(filename_list)

    team_chart(filename_list)
