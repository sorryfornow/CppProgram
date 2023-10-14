#include <map>
#include <set>
#include <list>
#include <cmath>
#include <ctime>
#include <deque>
#include <queue>
#include <stack>
#include <string>
#include <bitset>
#include <cstdio>
#include <limits>
#include <vector>
#include <climits>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <numeric>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>

using ClientId = uint32_t;
using TruckId = uint32_t;

struct TruckPosition
{
    double X;
    double Y;
};

struct TruckPositionDelta
{
    TruckId mTruckId;
    double DeltaX;
    double DeltaY;
};

class IServer
{
public:
    virtual ~IServer() {}
    virtual TruckPosition SubscribeToTruck(TruckId) = 0;
};

class ISubscriber
{
public:
    virtual ~ISubscriber() {}

    // Called by server after initial subscription
    virtual void ProcessUpdate(const TruckPositionDelta&) = 0;

    // Called by clients
    virtual TruckPosition SubscribeToTruck(TruckId, ClientId) = 0;
    virtual std::vector<TruckPositionDelta> GetUpdates(ClientId) = 0;
};

class Subscriber : public ISubscriber
{
public:
    Subscriber(IServer& server) : mServer(server) {}

    void ProcessUpdate(const TruckPositionDelta& delta) override {
        std::cout << "U " << delta.mTruckId << " " << delta.DeltaX+mServer.SubscribeToTruck(delta.mTruckId).X << " " << delta.DeltaY+mServer.SubscribeToTruck(delta.mTruckId).Y << std::endl;
    }
    TruckPosition SubscribeToTruck(TruckId Tid, ClientId Cid) override {
        mSubscriptions[Cid].insert(Tid);
        return mServer.SubscribeToTruck(Tid);
        }
    std::vector<TruckPositionDelta> GetUpdates(ClientId Cid) override {
        std::vector<TruckPositionDelta> res{};
        for (auto& truck : mSubscriptions.at(Cid))
        {
            TruckPositionDelta delta{truck,mServer.SubscribeToTruck(truck).X,mServer.SubscribeToTruck(truck).Y};
            res.emplace_back(delta);
        }

        return res;
    }

private:
    IServer& mServer;
    std::unordered_map<ClientId, std::unordered_set<TruckId>> mSubscriptions;
};

class Server : public IServer
{
public:
    TruckPosition SubscribeToTruck(TruckId id) override
    {
        mRegisteredTrucks.insert(id);
        return mCurrentPos.at(id);
    }

    void AddPosition(TruckId id, TruckPosition pos)
    {
        mCurrentPos.insert({id, pos});
    }

    void OnUpdate(Subscriber& subscriber, const TruckPositionDelta& delta)
    {
        if (mRegisteredTrucks.find(delta.mTruckId) != mRegisteredTrucks.end())
        {
            subscriber.ProcessUpdate(delta);
        }
        auto& pos = mCurrentPos.at(delta.mTruckId);
        pos.X += delta.DeltaX;
        pos.Y += delta.DeltaY;
    }

private:
    std::unordered_set<TruckId> mRegisteredTrucks;
    std::unordered_map<TruckId, TruckPosition> mCurrentPos;
};

class Client
{
public:
    Client(ClientId id, ISubscriber& subscriber) : mId(id), mSubscriber(subscriber) {}

    void Subscribe(TruckId truckId)
    {
        TruckPosition pos = mSubscriber.SubscribeToTruck(truckId, mId);
        std::cout << "S " << mId << " " << truckId << " " << pos.X << " " << pos.Y << std::endl;
    }

    void RequestUpdate()
    {
        std::vector<TruckPositionDelta> updates = mSubscriber.GetUpdates(mId);
        for (const TruckPositionDelta& delta : updates)
        {
            std::cout << "U " << mId << " " << delta.mTruckId << " " << delta.DeltaX << " " << delta.DeltaY << std::endl;
        }
    }
private:
    ClientId mId;
    ISubscriber& mSubscriber;
};

std::vector<std::string> split(const std::string& input)
{
    std::vector<std::string> result;
    std::stringstream stringStream (input);
    std::string item;

    while (std::getline(stringStream, item, ' '))
        result.push_back(item);

    return result;
}

int main()
{
    Server server;
    Subscriber subscriber(server);
    std::vector<Client> clients;

    std::string input;

    // Read number of trucks
    std::getline(std::cin, input);
    uint32_t numTrucks = std::stoi(input);
    for (uint32_t i = 0; i < numTrucks; i++)
    {
        std::getline(std::cin, input);
        std::vector<std::string> position = split(input);
        server.AddPosition(i, {std::stod(position[0]), std::stod(position[1])});
    }

    while(std::getline(std::cin, input))
    {
        std::vector<std::string> command = split(input);
        switch(command[0][0])
        {
        case 'S':
            {
                uint32_t clientId = std::stoi(command[1]);
                if (clientId >= clients.size())
                {
                    clients.push_back(Client(clientId, subscriber));
                }
                clients.at(clientId).Subscribe(std::stoi(command[2]));
            }
            break;
        case 'U':
            server.OnUpdate(subscriber, {static_cast<TruckId>(std::stoi(command[1])), std::stod(command[2]), std::stod(command[3])});
            break;
        case 'R':
            clients.at(std::stoi(command[1])).RequestUpdate();
            break;
        default:
            throw std::runtime_error("Invalid input");
        }
    }

    return 0;
}
