import ijson
import numpy as np
import matplotlib
import matplotlib.pyplot as plt
import time

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
    for i in range(1, 2):  # TODO: change to taken spots -> player name required
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


def plot_stats_category(filename_list, category, n_xticks=None):
    fig, ax = plt.subplots() 

    # plot curves
    data = stack_values(filename_list, category)
    x = combine_ticks(filename_list)
    for i in range(data.shape[0]):
        y = data[i][:]
        ax.plot(x, y, label='Player'+str(i))  # TODO: player name 

    # formatting
    formatter = matplotlib.ticker.FuncFormatter(lambda datapoint, x: time.strftime('%H:%M', time.gmtime(round(datapoint / 15.63))))
    ax.xaxis.set_major_formatter(formatter)
    if n_xticks:
        ax.xaxis.set_major_locator(plt.MaxNLocator(n_xticks))
    ax.legend()
    ax.set_title(category)
    ax.set_ylim(bottom=0)
    ax.set_xlim(left=0)
    plt.show()


def stack_chart():
    x=range(1,6)
    y1=[1,4,6,8,9]
    y2=[2,2,7,10,12]
    y3=[2,8,5,10,6]

    # Basic stacked area chart.
    plt.stackplot(x,y1, y2, y3, labels=['A','B','C'])
    plt.legend()
    plt.show()


if __name__ == '__main__':
    # testrecording
    filename_list = ['example/s3-2021-04-20-08-25-29.json']
    plot_stats_category(filename_list, category='buildings')
    plot_stats_category(filename_list, category='soldiers')

    # stack_chart()
