import time
import ijson
import numpy as np
import matplotlib
import matplotlib.pyplot as plt
import matplotlib.patheffects as pe
from cycler import cycler

# apply custom style sheet globally
plt.style.use('custom-dark')


def get_tick(filename):
    with open(filename, 'rb') as f:
        object = ijson.items(f, 'stats.tick')
        for data in object:
            return data
            
            
# get json data of player i by property ['name', 'team', 'race', 'score', 'settlers', 'buildings', 'food', 'mines', 'gold', 'manna', 'soldiers', 'battles']
def get_player_values(filename, player_number, property):
    with open(filename, 'rb') as f:
        object = ijson.items(f, 'stats.player' + str(player_number) + '.' + property)
        for data in object:
            return data


# get json about game by property ['date', 'time', 'numberOfPlayers', 'map', 'rules']
def get_general_values(filename, property):
    with open(filename, 'rb') as f:
        object = ijson.items(f, 'general.' + property)
        for data in object:
            return data


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
        values = np.array(get_player_values(filename, player_number, property), dtype=np.int64)
        combined_values = np.append(combined_values, values, axis=0)

    return combined_values


# stack all players to 2d-array
def stack_values(filename_list, property):
    stacked_values = combine_values(filename_list, 0, property)
    for i in range(1, 4):  # TODO: change to taken spots -> player name required
        combined_values = combine_values(filename_list, i, property)
        if np.count_nonzero(combined_values) > 0 or i <= 1:  # TODO: change to taken spots -> player name required
            stacked_values = np.vstack((stacked_values, combined_values))

    # clean-up
    # while(1):
    #     cleanup_reference = np.sum(stacked_values, axis=0)
    #     zero_indices = np.where(cleanup_reference == 0)
    #     startpoint = np.amax(zero_indices) + 1  # startpoint of the last save
    #     startpoint_value = cleanup_reference[startpoint]
    #     possible_savepoints = np.where(cleanup_reference[:startpoint] >= startpoint_value)[0]  # find approximate save time
    #     if possible_savepoints.size == 0:
    #         break
    #     savepoint = possible_savepoints[0]
    #     stacked_values = np.delete(stacked_values, np.arange(savepoint, startpoint), axis=1)

    return stacked_values


def plot_stats(filename_list, property, n_xticks=None):
    data = stack_values(filename_list, property)
    x = combine_ticks(filename_list)

    # plot curves
    fig, ax = plt.subplots() 
    for i in range(data.shape[0]):
        y = data[i][:]
        ax.plot(x, y, label='Player'+str(i))  # TODO: player name 

    # formatting
    formatter = matplotlib.ticker.FuncFormatter(lambda datapoint, x: time.strftime('%H:%M', time.gmtime(round(datapoint / 15.63))))
    ax.xaxis.set_major_formatter(formatter)
    if n_xticks:
        ax.xaxis.set_major_locator(plt.MaxNLocator(n_xticks))
    ax.legend()
    ax.set_title(property)
    ax.set_ylim(bottom=0)
    ax.set_xlim(left=0)
    plt.show()


def stack_chart(filename_list):
    soldiers = stack_values(filename_list, property='soldiers') + 21  # TODO: start soldiers depending on goods 
    soldiers_sum = np.sum(soldiers, axis=0)
    soldiers_rel = np.flip(soldiers / soldiers_sum, axis=0)
    x = combine_ticks(filename_list)
    number_of_players = soldiers.shape[0]
    player_names = ['Player'+str(i) for i in range(number_of_players)]  # TODO: player names
    
    # flip color cycle
    colors = plt.rcParams["axes.prop_cycle"].by_key()["color"]

    # plot stack chart
    fig, ax = plt.subplots() 
    plt.stackplot(x, soldiers_rel, labels=player_names, colors=colors[:number_of_players][::-1], alpha=0.75)
    for i in range(1, number_of_players):
        y = np.sum(soldiers_rel[:i][:], axis=0)
        plt.plot(x, y, linewidth=0.8, color='white', alpha=0.7)  # white lines between stacks

    # formatting
    ax.set_title('Players Soldier Percentage')
    ax.set_ylim(bottom=0, top=1)
    ax.set_xlim(left=0, right=x[-1]) 
    formatter = matplotlib.ticker.FuncFormatter(lambda datapoint, x: time.strftime('%H:%M', time.gmtime(round(datapoint / 15.63))))
    ax.xaxis.set_major_formatter(formatter)
    ax.yaxis.set_major_locator(plt.MaxNLocator(4))
    ax.yaxis.set_ticklabels([])
    #ax.set_position([0.1, 0.1, 0.7, 0.8])  # TODO enough space for legend

    # legend
    leg = ax.legend(loc='center right', bbox_to_anchor=(-0.01, 0.5))
    for i in range(number_of_players):
        leg.legendHandles[i].set_color(colors[i])
    ax.add_artist(leg)

    # vertical manna lines
    manna = np.flip(stack_values(filename_list, property='manna'), axis=0)
    for i in range(number_of_players):
        manna_l2 = np.where(manna[i][:] >= 10)
        if np.any(manna_l2):
            l2_index = manna_l2[0][:][0]
            ymin = 0
            for j in range(i):
                ymin += soldiers_rel[j][l2_index]
            plt.vlines(x[l2_index], ymin=ymin, ymax=ymin+soldiers_rel[i][l2_index], linewidth=0.8, colors='white', 
                       path_effects=[pe.SimpleLineShadow(shadow_color='grey'), pe.Normal()])

        manna_l3 = np.where(manna[i][:] >= 110)
        if np.any(manna_l3):
            l3_index = manna_l3[0][:][0]
            ymin = 0
            for j in range(i):
                ymin += soldiers_rel[j][l3_index]
            plt.vlines(x[l3_index], ymin=ymin, ymax=ymin+soldiers_rel[i][l3_index], linewidth=0.8, colors='white',
                       path_effects=[pe.SimpleLineShadow(shadow_color='grey'), pe.Normal()])

    plt.show()


if __name__ == '__main__':
    # testrecording
    filename_list = ['example/s3-2021-04-26-02-46-37.json']
    # plot_stats(filename_list, property='buildings')
    # plot_stats(filename_list, property='soldiers')

    stack_chart(filename_list)
