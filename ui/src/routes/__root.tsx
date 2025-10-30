import { Outlet, createRootRoute } from '@tanstack/react-router';
import { SidebarProvider, SidebarInset } from '@/components/ui/sidebar';
import { AppSidebar } from '@/components/layout/sidebar';

export const Route = createRootRoute({
  component: () => (
    <>
      <SidebarProvider>
        <AppSidebar />
        <SidebarInset className='overflow-hidden'>
          <Outlet />
        </SidebarInset>
      </SidebarProvider>
    </>
  )
});
