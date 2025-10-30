import { Link } from '@tanstack/react-router';

import {
  Egg,
  LayoutDashboard,
  MonitorSmartphone,
  Network,
  Settings,
  Wifi
} from 'lucide-react';

import {
  Sidebar,
  SidebarHeader,
  SidebarContent,
  SidebarGroup,
  SidebarGroupContent,
  SidebarMenu,
  SidebarMenuButton,
  SidebarMenuItem,
  useSidebar
} from '@/components/ui/sidebar';

export const menuItems = [
  {
    title: 'Dashboard',
    url: '/dashboard',
    icon: LayoutDashboard
  },
  {
    title: 'Wireless',
    url: '#',
    icon: Wifi
  },
  {
    title: 'Clients',
    url: '#',
    icon: MonitorSmartphone
  },
  {
    title: 'Network',
    url: '#',
    icon: Network
  },
  {
    title: 'Settings',
    url: '/settings',
    icon: Settings
  }
];

export function AppSidebar() {
  const { setOpenMobile } = useSidebar();

  return (
    <Sidebar collapsible='icon'>
      <SidebarHeader className='h-14 border-b flex flex-row items-center'>
        <SidebarMenu>
          <SidebarMenuItem>
            <SidebarMenuButton
              className='hover:bg-transparent active:bg-transparent flex'
              asChild
            >
              <div>
                <Egg />
                <span>ESP Dragon Fruit</span>
              </div>
            </SidebarMenuButton>
          </SidebarMenuItem>
        </SidebarMenu>
      </SidebarHeader>
      <SidebarContent>
        <SidebarGroup>
          <SidebarGroupContent>
            <SidebarMenu>
              {menuItems.map((item) => (
                <SidebarMenuItem key={item.title}>
                  <SidebarMenuButton asChild>
                    <Link
                      onClick={() => setOpenMobile(false)}
                      to={item.url}
                      activeProps={{
                        className: 'text-sky-800 hover:text-sky-800'
                      }}
                    >
                      <item.icon />
                      <span>{item.title}</span>
                    </Link>
                  </SidebarMenuButton>
                </SidebarMenuItem>
              ))}
            </SidebarMenu>
          </SidebarGroupContent>
        </SidebarGroup>
      </SidebarContent>
    </Sidebar>
  );
}
