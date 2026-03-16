#include "../INC/Server.hpp"
/*
RPL_INVITING (341)
ERR_NEEDMOREPARAMS (461)
ERR_NOSUCHCHANNEL (403)
ERR_NOTONCHANNEL (442)
ERR_CHANOPRIVSNEEDED (482)
ERR_USERONCHANNEL (443)*/
void Server::Invite(std::string &cmd, int &fd)
{
	std::vector<std::string> scmd = split_cmd(cmd);
	if(scmd.size() < 3)// ERR_NEEDMOREPARAMS (461) if there are not enough parameters
		{_sendResponse(ERR_NEEDMOREPARAMS(std::string("INVITE")), fd); return;}
	std::string channelname = scmd[2].substr(1);
	if(scmd[2][0] != '#' || !GetChannel(channelname))// ERR_NOSUCHCHANNEL (403) if the given channel does not exist
	    {_sendResponse(ERR_NOSUCHCHANNEL(GetClient(fd)->GetNickName(), channelname), fd); return;}
	if (!(GetChannel(channelname)->get_client(fd)) && !(GetChannel(channelname)->get_admin(fd)))// ERR_NOTONCHANNEL (442) if the client is not on the channel
	    {_sendResponse(ERR_NOTONCHANNEL(GetClient(fd)->GetNickName(), channelname), fd); return;}
	if (GetChannel(channelname)->GetClientInChannel(scmd[1]))// ERR_USERONCHANNEL (443) if the given nickname is already on the channel
	    {_sendResponse(ERR_USERONCHANNEL(GetClient(fd)->GetNickName(), channelname), fd); return;}
	Client *clt = GetClientNick(scmd[1]);
	if (!clt)// ERR_NOSUCHNICK (401) if the given nickname is not found
		{_sendResponse(ERR_NOSUCHNICK(channelname, scmd[1]), fd);return;}
	if (GetChannel(channelname)->GetInvitOnly() && !GetChannel(channelname)->get_admin(fd))// ERR_INVITEONLYCHAN (473) if the channel is invite-only
		{_sendResponse(ERR_INVITEONLYCHAN(GetClient(fd)->GetNickName(), channelname), fd); return;}
	if (GetChannel(channelname)->GetLimit() && GetChannel(channelname)->GetClientsNumber() >= GetChannel(channelname)->GetLimit()) // ERR_CHANNELISFULL (471) if the channel is full
	{_sendResponse(ERR_CHANNELISFULL(GetClient(fd)->GetNickName(), channelname), fd); return;}
	// RPL_INVITING (341) if the invite was successfully sent
	clt->AddChannelInvite(channelname);
//	std::string rep1 = ": 341 "+ GetClient(fd)->GetNickName()+" "+ clt->GetNickName()+" "+ scmd[2]+"\r\n";
//	_sendResponse(rep1, fd);
	_sendResponse(RPL_INVITING(GetClient(fd)->GetNickName(), clt->GetNickName(), scmd[2]), fd);
	_sendResponse(RPL_INVITING(GetClient(fd)->GetNickName(), clt->GetNickName(), scmd[2]), clt->GetFd());
//	_sendResponse(rep2, clt->GetFd());
	
}